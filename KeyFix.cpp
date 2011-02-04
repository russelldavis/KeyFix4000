#include "stdafx.h"
#include "KeyFix.h"
#include <assert.h>
#include <psapi.h>


HHOOK CKeyFix::m_hHook = NULL;
bool CKeyFix::m_bUsed = false;
bool CKeyFix::m_bDown = false;
bool CKeyFix::m_bIgnore = false;
bool CKeyFix::m_bLeftDown = false;
bool CKeyFix::m_bRightDown = false;

bool IsKeyPressedAsync(int vKey)
{
	// MSB is the down-state
	return (GetAsyncKeyState(vKey) & 0x8000) == 0x8000;
}

bool IsAnyModifierDown()
{
	return IsKeyPressedAsync(VK_CONTROL) ||
	       IsKeyPressedAsync(VK_SHIFT) ||
		   IsKeyPressedAsync(VK_MENU);
}

LRESULT CALLBACK CKeyFix::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (!m_bIgnore && nCode >= 0)
	{
		PKBDLLHOOKSTRUCT pkb = PKBDLLHOOKSTRUCT (lParam);
#ifdef DEBUG
		if (!(pkb->flags & LLKHF_UP))
		{
			char buf[100];
			_itoa(pkb->scanCode, buf, 10);
			OutputDebugString(buf);
		}
#endif
		if (pkb->vkCode == VK_BROWSER_BACK)
		{
			DWORD dwFlags = 0;
			if (pkb->flags & LLKHF_UP)
			{
				dwFlags = MOUSEEVENTF_LEFTUP;
				m_bLeftDown = false;
			}
			else
			{
				// Key repeat will generate keydown events when the user is just holding
				// the button. We don't want to click again when that happens.
				if (m_bLeftDown)
					return 1;

				dwFlags = MOUSEEVENTF_LEFTDOWN;
				m_bLeftDown = true;
			}

			mouse_event(dwFlags, 0, 0, 0, NULL);
			return 1;
		}
		else if (pkb->vkCode == VK_BROWSER_FORWARD)
		{
			DWORD dwFlags = 0;
			if (pkb->flags & LLKHF_UP)
			{
				dwFlags = MOUSEEVENTF_RIGHTUP;
				m_bRightDown = false;
			}
			else
			{
				// Key repeat will generate keydown events when the user is just holding
				// the button. We don't want to click again when that happens.
				if (m_bRightDown)
					return 1;
				dwFlags = MOUSEEVENTF_RIGHTDOWN;
				m_bRightDown = true;
			}

			mouse_event(dwFlags, 0, 0, 0, NULL);
			return 1;
		}
		else if (pkb->vkCode == VK_APPS)
		{
			// Checking the actual flag rather than m_bDown here, in case
			// multiple keydowns happen before a keyup.
			if (pkb->flags & LLKHF_UP)
			{
				m_bDown = false;
				if (m_bUsed)
				{
					// We are treating this as the winkey, so kill the appskey-down
					// and send a winkey-down instead.
					m_bUsed = false;

					m_bIgnore = true;
					keybd_event(VK_RWIN, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, NULL);
					m_bIgnore = false;

					return 1;
				} else {
					// Send the keydown first (since we swallowed the original keydown event)
					m_bIgnore = true;
					keybd_event(VK_APPS, 0, KEYEVENTF_EXTENDEDKEY /* KEYDOWN BY DEFAULT */, NULL);
					m_bIgnore = false;
				}
			}
			else
			{
				// Visual Assist X has a menu activated by shift+appskey-down. To allow that
				// to work, we must disable special processing of the appskey when shift is
				// down. This of course means shift+appskey won't ever be translated to
                // shift+winkey.
				if (!IsKeyPressedAsync(VK_SHIFT))
				{
					m_bDown = true;
					// Don't send the keydown, we don't know how the key will be used yet.
					return 1;
				}
			}
		}
		else if (m_bDown && !m_bUsed)
		{
			// User pressed another key while apps key was down. We now want to treat
			// the key as the winkey, so send a winkey-down.
			m_bUsed = true;

			m_bIgnore = true;
			keybd_event(VK_RWIN, 0, KEYEVENTF_EXTENDEDKEY, NULL);
			m_bIgnore = false;
		}

	}
	return CallNextHookEx(m_hHook, nCode, wParam, lParam);
}

void CKeyFix::SetHook()
{
	m_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, &LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
}

void CKeyFix::RemoveHook()
{
	if (m_hHook)
	{
		UnhookWindowsHookEx(m_hHook);
		m_hHook = NULL;
	}
}
