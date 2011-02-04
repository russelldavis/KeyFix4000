#include <vector>
#include "windows.h"

class CKeyFix
{
public:
	static void SetHook();
	static void RemoveHook();

private:

	static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
	static void HandleHostKeyUp();
	static void HandleHostKeyDown();

	static HHOOK m_hHook;
	static bool m_bDown;
	static bool m_bUsed;
	static bool m_bIgnore;

	static bool m_bHostKeyDown;
	static bool m_bLeftDown;
	static bool m_bRightDown;

};
