#pragma once

class BaseUI {
public:
	virtual void RenderUI(std::function<void()> callback = nullptr);
};