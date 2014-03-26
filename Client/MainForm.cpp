#include "MainForm.h"

#include <Windows.h>

#include"include\pa_asio.h"
#include"include\pa_win_ds.h"
#include"include\pa_win_wasapi.h"
#include"include\pa_win_waveformat.h"
#include"include\pa_win_wdmks.h"
#include"include\pa_win_wmme.h"
#include"include\portaudio.h"

using namespace System;
using namespace System::Windows::Forms;
using namespace System::Data;

[STAThread]
void main(array<String^>^ args) {
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);

    Client::MainForm form;
    Application::Run(%form);

	

	Pa_Initialize();


	Pa_Terminate();

}