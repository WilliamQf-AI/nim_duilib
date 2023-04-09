// popover.cpp : ����Ӧ�ó������ڵ㡣
//

#include "framework.h"
#include "popover_form.h"

enum ThreadId
{
  kThreadUI
};

class MainThread : public nbase::FrameworkThread
{
public:
  MainThread() : nbase::FrameworkThread("MainThread") {}
  virtual ~MainThread() {}

private:

  virtual void Init() override
  {
	nbase::ThreadManager::RegisterThread(kThreadUI);

	// ��ȡ��Դ·������ʼ��ȫ�ֲ���
	std::wstring theme_dir = nbase::win32::GetCurrentModuleDirectory();

	ui::GlobalManager::EnableAutomation(true);
	ui::GlobalManager::Startup(theme_dir + L"resources\\", ui::CreateControlCallback(), true);


	// ����һ��Ĭ�ϴ�����Ӱ�ľ��д���
	PopoverForm* window = new PopoverForm();
	window->Create(NULL, PopoverForm::kClassName.c_str(), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0);
	window->CenterWindow();
	window->ShowWindow();
  }

  /**
  * �麯�������߳��˳�ʱ����һЩ��������
  * @return void	�޷���ֵ
  */
  virtual void Cleanup() override
  {
    ui::GlobalManager::Shutdown();
    SetThreadWasQuitProperly(true);
    nbase::ThreadManager::UnregisterThread();
  }
};


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	// �������߳�
	MainThread thread;

	// ִ�����߳�ѭ��
	thread.RunOnCurrentThreadWithLoop(nbase::MessageLoop::kUIMessageLoop);

    return 0;
}