#pragma once
#include "BaseUI.h"

class WindowsUI : public BaseUI {
public:
	WindowsUI();
	~WindowsUI();

	void RenderUI(std::function<void()> callback = nullptr) override;
protected:

};