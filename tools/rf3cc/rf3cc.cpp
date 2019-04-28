// simple command line tool for processing rf3 files

#include "stdafx.h"

#include <atlbase.h>
#include <comdef.h>

#include "RwUtilTypes.h"
#include "RwUtilString.h"

#include "RwExpExportOptions.h"

#include "RwRf3Translator_i.c"
#include "RwRf3Translator.h"

#include "RwCmdLineMessage.h"

// check if string is a valid boolean flag
bool
CheckFlag(const RwWString sFlag, bool &result)
{
    if (_wcsicmp(sFlag.c_str(), L"true") == 0)
    {
        result = true;
    }
    else if (_wcsicmp(sFlag.c_str(), L"false") == 0)
    {
        result = false;
    }    
    else
    {
        return false;
    }

    return true;
}

// nice and simple
int
wmain(int argc, wchar_t* argv[])
{
    wchar_t buffer[256];
    RwWString sProjectTemplate = L"Generic.rwt";
    RwWString sProjectName = L"untitled";
    RwWString sProjectPath = L"";
    bool processFlags = true;
    bool nameSet = false;
    RwVector<RwWString> vFiles;
    int argIndex = 1;
    bool badusage = false;
    IRf3ExportOptions *pExportOptions = NULL;
    HRESULT hResult;
	RwCmdLineMessage cmdLineMessage;
	IConnectionPointContainer *pCPC = NULL;
	IConnectionPoint * pCP = NULL;

    hResult = ::CoInitialize(NULL);

    if (!SUCCEEDED(hResult = ::CoCreateInstance(CLSID_Rf3ExportOptions, NULL, CLSCTX_ALL, IID_IRf3ExportOptions, reinterpret_cast<void**>(&pExportOptions))))
    {
		CoUninitialize();
        return -1;
    }
    
    // get the current path as the default project path
    if (GetCurrentDirectoryW(256, buffer) != 0)
    {
        sProjectPath =  buffer;
    }

    wprintf(L"Current Directory: %s\r\n", buffer);

    if( argc > 1 )
    {
        // process the command line flags
        while ((processFlags) && (argIndex < argc))
        {                  
            RwWString sFlag = argv[argIndex];

            if (sFlag == L"-t") // project template file
            {
                argIndex++;
                sProjectTemplate = argv[argIndex];
                argIndex++;
            }
            else if (sFlag == L"-n") // project name
            {
                argIndex++;
                sProjectName = argv[argIndex];
                argIndex++;
                nameSet = true;
            }
            else if (sFlag == L"-p") // export path
            {
                argIndex++;
                sProjectPath = argv[argIndex];
                argIndex++;
            }
            else if (sFlag == L"-expold")
            {              
                argIndex++;
                RwWString sOldFlag = argv[argIndex];
                argIndex++;
                bool expold;

                if (CheckFlag(sOldFlag, expold))
                {  
                    //outputOptions.SetOption(RwCommOption(EXPORTTOLEGACYFILES, expold));
                    pExportOptions->SetOption(_bstr_t(EXPORTTOLEGACYFILES), CComVariant(expold));

                }
                else
                {
                    badusage = true;
                    printf("BAD USAGE: the \"-expold\" flag requires a boolean value. use either\r\n");
                    printf("           \"true\" or \"false\"\r\n\r\n");
                }
            }
            else if (sFlag == L"-exprws")
            {   
                argIndex++;
                RwWString sRWSFlag = argv[argIndex];
                argIndex++;
                bool expRWS;

                if (CheckFlag(sRWSFlag, expRWS))
                {  
                    //outputOptions.SetOption(RwCommOption(EXPORTTORWS, expRWS));
                    pExportOptions->SetOption(_bstr_t(EXPORTTORWS), CComVariant(expRWS));
                }
                else
                {
                    badusage = true;
                    printf("BAD USAGE: the \"-exprws\" flag requires a boolean value. use either\r\n");
                    printf("           \"true\" or \"false\"\r\n\r\n");                    
                }
            }          
            else
            {
                processFlags = false;
            }
        }

        if (!badusage)
        {
            // set the project name to the first rf3 file's name by default
            if (!nameSet)
            {
                sProjectName = argv[argIndex];
                RwUtilStripToFileName(sProjectName);
                RwUtilStripOffExtension(sProjectName);
            }
           
            // create a list of the rf3 files
            for (int i = argIndex; i < argc; i++)
            {
                RwWString cleanFilePath;
                RwWString sRf3File = argv[i];
                RwUtilStripOffExtension(sRf3File);
                sRf3File += L".rf3";

                cleanFilePath = sRf3File;

                // convert to absolute path
                if (PathIsRelativeW(sRf3File.c_str()))
                {
                    // create absolute
                    cleanFilePath = RwWString(buffer) + L"\\" + sRf3File;

                    // check if path exists
                    if (!PathFileExistsW(cleanFilePath.c_str()))
                    {
                        // no... use project instead
                        cleanFilePath = sProjectPath + L"\\" + sRf3File;
                
                        // check if path exists
                        if (!PathFileExistsW(cleanFilePath.c_str()))
                        {
                            // no.. leave it as it was
                            cleanFilePath = sRf3File;
                        }
                    }
                }

                if (PathFileExistsW(cleanFilePath.c_str()))
                {
                    vFiles.push_back(cleanFilePath);
                }
                else
                {
                    printf("Error: RF3 file does not exist! %s\r\n", sRf3File.c_str());
                }
            }

            if (!vFiles.empty())
            {
                IRf3Translator *pTranslator = NULL;

                // export the rf3 files
                //eResult = RwExpMgrExportManager::TheInstance()->Export(vFiles,
                //    sProjectTemplate, sProjectName, sProjectPath, &outputOptions);

                HRESULT hResult = CoCreateInstance(CLSID_Rf3Translator, NULL, CLSCTX_ALL, IID_IRf3Translator, reinterpret_cast<void**>(&pTranslator) );
                if (SUCCEEDED(hResult))
                {
					// Connect a command line message event
					// First check if the interface supports connectable objects
					HRESULT hr = pTranslator->QueryInterface(IID_IConnectionPointContainer,(void **)&pCPC);
					if ( SUCCEEDED(hr) )
					{
						// Get the correct connection point interface
						hr = pCPC->FindConnectionPoint(IID_IRf3MessageEvents,&pCP);
						if ( SUCCEEDED(hr) )
						{
							//Get the pointer to RwCmdLineMessage's IUnknown pointer
							IUnknown *pSinkUnk;
							hr = cmdLineMessage.QueryInterface(IID_IUnknown,(void **)&pSinkUnk);

							// Pass it to the COM object through IRf3MessageEvents
							DWORD dwAdvise;
							hr = pCP->Advise(pSinkUnk,&dwAdvise);

							// The COM object IRf3MessageEvents are now connected to cmdLineMessage
						}
					}
					if ( !SUCCEEDED(hr) )
					{
						wprintf(L"Could not connection to Rf3Translator messages.\r\n");
					}
                    
                    CComVariant rf3Files(VT_BSTR | VT_ARRAY);
                    BSTR HUGEP *pbstr;

                    // Create the rf3 file list
                    rf3Files.vt = (VT_BSTR | VT_ARRAY);
                    rf3Files.parray = ::SafeArrayCreateVector(VT_BSTR, 0, vFiles.size());

                    // Get a pointer to the elements of the array.
                    hResult = ::SafeArrayAccessData(rf3Files.parray, (void HUGEP**)&pbstr);
                    for (unsigned i = 0; i < vFiles.size(); i++)
                    {
                        _bstr_t t(vFiles[i].data());
                        pbstr[i] = ::SysAllocString(t);
                    }



                    hResult = pTranslator->Translate(rf3Files,
                                            _bstr_t(sProjectTemplate.data()),
                                            _bstr_t(sProjectName.data()),
                                            _bstr_t(sProjectPath.data()),
                                            pExportOptions);

                    pTranslator->Release();

                    if (pExportOptions)
                    {
                        pExportOptions->Release();
                    }

                    // Free up array
                    ::SafeArrayUnaccessData(rf3Files.parray);
					// NOTE: CComVariant handles destruction of the safearray
                }

                if( !SUCCEEDED(hResult) )
                {
                    printf("\r\nError occured while building rf3!\r\n");
                }
            }
        }
    }
    else
    {
        badusage = true;
    }

    // print help if we have a bad usage of the tool
    if (badusage)
    {
        RwWString appName = argv[0];
        RwUtilStripToFileName(appName);
        RwUtilStripOffExtension(appName);

        wprintf(L"%s : RenderWare RF3 file commandline compiler\r\n", appName.c_str());
        wprintf(L"        (c) 2003 Criterion Software Ltd.\r\n\r\n");
        wprintf(L"%s usage :\r\n", appName.c_str());
        wprintf(L"%s <options>... <rf3 files>...\r\n", appName.c_str());
        wprintf(L"options :\r\n");
        wprintf(L"-t <template>        The project template file\r\n");
        wprintf(L"-n <name>            The project name (default rws filename)\r\n");
        wprintf(L"-p <path>            The export path\r\n");
        wprintf(L"-expold <true/false> This flag overrides the Export Legacy files\r\n");
        wprintf(L"                     flag in the project template\r\n");
        wprintf(L"-exprws <true/false> This flag overrides the Export RWS file\r\n");
        wprintf(L"                     flag in the project template\r\n\r\n");
        wprintf(L"examples :\r\n");
        wprintf(L"   %s cube.rf3\r\n", appName.c_str());
        wprintf(L"       export cube.rf3 assets to the same directory\r\n");
        wprintf(L"   %s -t CubeScene.rwt -n CubeOnTable -p c:\\ cube.rf3 table.rf3\r\n", appName.c_str());
        wprintf(L"       export both cube.rf3 and table.rf3 assets to c:\\CubeOnTable.rws using\r\n");
        wprintf(L"       the CubeScene.rwt template\r\n\r\n");
        wprintf(L"For more information on %s please refer to the rf3cc user guide.\r\n", appName.c_str());

		CoUninitialize();
        return 1;
    }

    CoUninitialize();
	return 0;
}
