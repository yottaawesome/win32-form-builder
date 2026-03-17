export module designer:rendering;
import std;
import formbuilder;
import :state;
import :hit_testing;
import :helpers;

namespace Designer
{

	export void DrawSelection(const DesignState& state, Win32::HDC hdc)
	{
		if (state.selection.empty()) return;

		int offset = RulerOffset(state);
		auto accent = Win32::CreateSolidBrush(state.theme.selectionHighlight);
		auto locked = Win32::CreateSolidBrush(state.theme.lockedHighlight);

		for (int idx : state.selection)
		{
			if (idx < 0 || idx >= static_cast<int>(state.entries.size()))
				continue;

			auto& ctrl = *state.entries[idx].control;
			auto& r = ctrl.rect;
			auto brush = ctrl.locked ? locked : accent;
			int ox = r.x + offset;
			int oy = r.y + offset;

			Win32::RECT sides[] = {
				{ ox - 2, oy - 2,            ox + r.width + 2, oy },
				{ ox - 2, oy + r.height,     ox + r.width + 2, oy + r.height + 2 },
				{ ox - 2, oy,                ox,                oy + r.height },
				{ ox + r.width, oy,          ox + r.width + 2,  oy + r.height },
			};
			for (auto& s : sides)
				Win32::FillRect(hdc, &s, brush);
		}

		// Resize handles only when exactly one unlocked control is selected.
		int sel = SingleSelection(state);
		if (sel >= 0 && sel < static_cast<int>(state.entries.size()) &&
			!state.entries[sel].control->locked)
		{
			auto& r = state.entries[sel].control->rect;
			Win32::POINT anchors[8];
			GetHandleAnchors(r, anchors, state.dpiInfo);
			int hs = state.dpiInfo.HandleSize();

			auto handleBrush = Win32::CreateSolidBrush(state.theme.handleFill);
			for (auto& a : anchors)
			{
				Win32::RECT outer = { a.x + offset, a.y + offset,
									a.x + hs + offset, a.y + hs + offset };
				Win32::RECT inner = { a.x + 1 + offset, a.y + 1 + offset,
									a.x + hs - 1 + offset, a.y + hs - 1 + offset };
				Win32::FillRect(hdc, &outer, accent);
				Win32::FillRect(hdc, &inner, handleBrush);
			}
			Win32::DeleteObject(handleBrush);
		}

		Win32::DeleteObject(locked);
		Win32::DeleteObject(accent);

		// Draw group indicators on grouped controls.
		auto groupPen = Win32::CreatePen(Win32::PenStyles::Dash, 1, state.theme.userGuide);
		auto oldPen = Win32::SelectObject(hdc, groupPen);
		auto nullBrush = static_cast<Win32::HBRUSH>(Win32::GetStockObject(Win32::NullBrush));
		auto oldBrush = Win32::SelectObject(hdc, nullBrush);

		for (int i = 0; i < static_cast<int>(state.entries.size()); ++i)
		{
			auto& ctrl = *state.entries[i].control;
			if (ctrl.groupId == 0) continue;
			int ox = ctrl.rect.x + offset;
			int oy = ctrl.rect.y + offset;
			Win32::Rectangle(hdc, ox - 4, oy - 4,
				ox + ctrl.rect.width + 4, oy + ctrl.rect.height + 4);
		}

		Win32::SelectObject(hdc, oldBrush);
		Win32::SelectObject(hdc, oldPen);
		Win32::DeleteObject(groupPen);
	}

	export void DrawHiddenOverlays(const DesignState& state, Win32::HDC hdc)
	{
		int offset = RulerOffset(state);
		auto hatch = Win32::CreateHatchBrush(Win32::HatchBDiagonal, state.theme.lockedHighlight);
		auto oldBrush = Win32::SelectObject(hdc, hatch);
		auto nullPen = static_cast<Win32::HPEN>(Win32::GetStockObject(Win32::NullPen));
		auto oldPen = Win32::SelectObject(hdc, nullPen);
		int oldMode = Win32::SetBkMode(hdc, Win32::Bk_Transparent);

		for (int i = 0; i < static_cast<int>(state.entries.size()); ++i)
		{
			auto& ctrl = *state.entries[i].control;
			if (ctrl.visible) continue;
			int ox = ctrl.rect.x + offset;
			int oy = ctrl.rect.y + offset;
			Win32::Rectangle(hdc, ox, oy, ox + ctrl.rect.width, oy + ctrl.rect.height);
		}

		Win32::SetBkMode(hdc, oldMode);
		Win32::SelectObject(hdc, oldPen);
		Win32::SelectObject(hdc, oldBrush);
		Win32::DeleteObject(hatch);
	}

	export void DrawDisabledOverlays(const DesignState& state, Win32::HDC hdc)
	{
		int offset = RulerOffset(state);
		auto hatch = Win32::CreateHatchBrush(Win32::HatchHorizontal, Win32::MakeRgb(180, 180, 180));
		auto oldBrush = Win32::SelectObject(hdc, hatch);
		auto nullPen = static_cast<Win32::HPEN>(Win32::GetStockObject(Win32::NullPen));
		auto oldPen = Win32::SelectObject(hdc, nullPen);
		int oldMode = Win32::SetBkMode(hdc, Win32::Bk_Transparent);

		for (int i = 0; i < static_cast<int>(state.entries.size()); ++i)
		{
			auto& ctrl = *state.entries[i].control;
			if (ctrl.enabled) continue;
			int ox = ctrl.rect.x + offset;
			int oy = ctrl.rect.y + offset;
			Win32::Rectangle(hdc, ox, oy, ox + ctrl.rect.width, oy + ctrl.rect.height);
		}

		Win32::SetBkMode(hdc, oldMode);
		Win32::SelectObject(hdc, oldPen);
		Win32::SelectObject(hdc, oldBrush);
		Win32::DeleteObject(hatch);
	}

	export void DrawPicturePlaceholders(const DesignState& state, Win32::HDC hdc)
	{
		int offset = RulerOffset(state);
		int oldMode = Win32::SetBkMode(hdc, Win32::Bk_Transparent);
		auto oldColor = Win32::SetTextColor(hdc, Win32::MakeRgb(128, 128, 128));
		auto font = Win32::CreateFontW(
			state.dpiInfo.Scale(14), 0, 0, 0, 400, 0, 0, 0, 1, 0, 0, 0, 0, L"Segoe UI");
		auto oldFont = Win32::SelectObject(hdc, font);

		for (auto& entry : state.entries)
		{
			if (entry.control->type != FormDesigner::ControlType::Picture) continue;
			if (entry.control->imagePath.empty())
			{
				auto& r = entry.control->rect;
				Win32::RECT rc = { r.x + offset, r.y + offset,
					r.x + offset + r.width, r.y + offset + r.height };
				Win32::DrawTextW(hdc, L"[Picture]", -1, &rc,
					Win32::DrawTextFlags::Center | Win32::DrawTextFlags::VCenter
					| Win32::DrawTextFlags::SingleLine);
			}
		}

		Win32::SelectObject(hdc, oldFont);
		Win32::DeleteObject(font);
		Win32::SetTextColor(hdc, oldColor);
		Win32::SetBkMode(hdc, oldMode);
	}

	// Paints ruler contents into a memory DC (called by DrawRulers).
	void PaintRulersToBuffer(const DesignState& state, Win32::HDC memDC, int canvasW, int canvasH)
	{
		int rs = state.dpiInfo.RulerSize();
		int fontSize = state.dpiInfo.Scale(11);
		int minorTick = state.dpiInfo.Scale(10);
		int majorTick = state.dpiInfo.Scale(50);
		int minorTickH = state.dpiInfo.Scale(4);
		int majorTickH = state.dpiInfo.Scale(8);

		auto font = Win32::CreateFontW(
			fontSize, 0, 0, 0, 400, 0, 0, 0, 1, 0, 0, 0, 0, L"Segoe UI");
		auto oldFont = Win32::SelectObject(memDC, font);
		auto oldTextColor = Win32::SetTextColor(memDC, state.theme.rulerText);
		auto oldBkMode = Win32::SetBkMode(memDC, Win32::Bk_Transparent);

		auto rulerBrush = Win32::CreateSolidBrush(state.theme.rulerBackground);
		Win32::RECT topRuler = { rs, 0, canvasW, rs };
		Win32::RECT leftRuler = { 0, rs, rs, canvasH };
		Win32::RECT corner = { 0, 0, rs, rs };
		Win32::FillRect(memDC, &topRuler, rulerBrush);
		Win32::FillRect(memDC, &leftRuler, rulerBrush);
		Win32::FillRect(memDC, &corner, rulerBrush);
		Win32::DeleteObject(rulerBrush);

		auto borderPen = Win32::CreatePen(0, 1, state.theme.rulerBorder);
		auto oldPen = Win32::SelectObject(memDC, borderPen);
		Win32::MoveToEx(memDC, rs, 0, nullptr);
		Win32::LineTo(memDC, rs, canvasH);
		Win32::MoveToEx(memDC, 0, rs, nullptr);
		Win32::LineTo(memDC, canvasW, rs);
		Win32::SelectObject(memDC, oldPen);
		Win32::DeleteObject(borderPen);

		auto tickPen = Win32::CreatePen(0, 1, state.theme.rulerTick);
		oldPen = Win32::SelectObject(memDC, tickPen);

		for (int px = 0; px < canvasW - rs; px += minorTick)
		{
			int screenX = px + rs;
			if (px % majorTick == 0)
			{
				Win32::MoveToEx(memDC, screenX, rs - majorTickH, nullptr);
				Win32::LineTo(memDC, screenX, rs);
				auto label = std::to_wstring(px);
				Win32::TextOutW(memDC, screenX + 2, 1, label.c_str(), static_cast<int>(label.size()));
			}
			else
			{
				Win32::MoveToEx(memDC, screenX, rs - minorTickH, nullptr);
				Win32::LineTo(memDC, screenX, rs);
			}
		}

		for (int py = 0; py < canvasH - rs; py += minorTick)
		{
			int screenY = py + rs;
			if (py % majorTick == 0)
			{
				Win32::MoveToEx(memDC, rs - majorTickH, screenY, nullptr);
				Win32::LineTo(memDC, rs, screenY);
				auto label = std::to_wstring(py);
				Win32::TextOutW(memDC, 1, screenY + 2, label.c_str(), static_cast<int>(label.size()));
			}
			else
			{
				Win32::MoveToEx(memDC, rs - minorTickH, screenY, nullptr);
				Win32::LineTo(memDC, rs, screenY);
			}
		}

		Win32::SelectObject(memDC, oldPen);
		Win32::DeleteObject(tickPen);
		Win32::SetBkMode(memDC, oldBkMode);
		Win32::SetTextColor(memDC, oldTextColor);
		Win32::SelectObject(memDC, oldFont);
		Win32::DeleteObject(font);
	}

	// Paints cursor indicator lines into a memory DC.
	void PaintCursorIndicatorToBuffer(const DesignState& state, Win32::HDC memDC, int formX, int formY)
	{
		int rs = state.dpiInfo.RulerSize();
		auto pen = Win32::CreatePen(0, 1, state.theme.formBoundary);
		auto oldPen = Win32::SelectObject(memDC, pen);

		int sx = formX + rs;
		Win32::MoveToEx(memDC, sx, 0, nullptr);
		Win32::LineTo(memDC, sx, rs);

		int sy = formY + rs;
		Win32::MoveToEx(memDC, 0, sy, nullptr);
		Win32::LineTo(memDC, rs, sy);

		Win32::SelectObject(memDC, oldPen);
		Win32::DeleteObject(pen);
	}

	export void DrawRulers(const DesignState& state, Win32::HDC hdc)
	{
		if (!state.showRulers) return;

		Win32::RECT rc;
		Win32::GetClientRect(state.canvasHwnd, &rc);
		int w = rc.right;
		int h = rc.bottom;

		// Double-buffer: draw rulers + indicator to off-screen bitmap, then blit.
		auto memDC = Win32::CreateCompatibleDC(hdc);
		auto memBmp = Win32::CreateCompatibleBitmap(hdc, w, h);
		auto oldBmp = Win32::SelectObject(memDC, memBmp);

		PaintRulersToBuffer(state, memDC, w, h);

		if (state.lastCursorPos.x >= 0)
			PaintCursorIndicatorToBuffer(state, memDC, state.lastCursorPos.x, state.lastCursorPos.y);

		// Blit top ruler strip.
		int rs = state.dpiInfo.RulerSize();
		Win32::BitBlt(hdc, 0, 0, w, rs, memDC, 0, 0, Win32::SrcCopy);
		// Blit left ruler strip.
		Win32::BitBlt(hdc, 0, rs, rs, h - rs, memDC, 0, rs, Win32::SrcCopy);

		Win32::SelectObject(memDC, oldBmp);
		Win32::DeleteObject(memBmp);
		Win32::DeleteDC(memDC);
	}

	export void DrawRulerCursorIndicator(const DesignState& state, Win32::HDC hdc, int formX, int formY)
	{
		// Now handled inside DrawRulers() via the double buffer - kept for API compatibility.
		(void)state; (void)hdc; (void)formX; (void)formY;
	}

	export void DrawUserGuides(const DesignState& state, Win32::HDC hdc)
	{
		if (state.userGuides.empty() && state.dragMode != DragMode::CreateGuide) return;

		int offset = RulerOffset(state);
		Win32::RECT rc;
		Win32::GetClientRect(state.canvasHwnd, &rc);

		auto pen = Win32::CreatePen(Win32::PenStyles::Dash, 0, state.theme.userGuide);
		auto oldPen = Win32::SelectObject(hdc, pen);
		auto oldMode = Win32::SetBkMode(hdc, Win32::Bk_Transparent);

		for (auto& guide : state.userGuides)
		{
			if (guide.horizontal)
			{
				Win32::MoveToEx(hdc, 0, guide.position + offset, nullptr);
				Win32::LineTo(hdc, rc.right, guide.position + offset);
			}
			else
			{
				Win32::MoveToEx(hdc, guide.position + offset, 0, nullptr);
				Win32::LineTo(hdc, guide.position + offset, rc.bottom);
			}
		}

		// Draw the guide being dragged (preview line).
		if (state.dragMode == DragMode::CreateGuide && state.draggingGuidePos >= 0)
		{
			if (state.draggingGuideHorizontal)
			{
				Win32::MoveToEx(hdc, 0, state.draggingGuidePos + offset, nullptr);
				Win32::LineTo(hdc, rc.right, state.draggingGuidePos + offset);
			}
			else
			{
				Win32::MoveToEx(hdc, state.draggingGuidePos + offset, 0, nullptr);
				Win32::LineTo(hdc, state.draggingGuidePos + offset, rc.bottom);
			}
		}

		Win32::SetBkMode(hdc, oldMode);
		Win32::SelectObject(hdc, oldPen);
		Win32::DeleteObject(pen);
	}

	export void DrawAlignGuides(const DesignState& state, Win32::HDC hdc)
	{
		if (state.guides.empty()) return;

		int offset = RulerOffset(state);
		Win32::RECT rc;
		Win32::GetClientRect(state.canvasHwnd, &rc);

		auto pen = Win32::CreatePen(Win32::PenStyles::Dot, 0, state.theme.alignGuide);
		auto oldPen = Win32::SelectObject(hdc, pen);
		auto oldMode = Win32::SetBkMode(hdc, Win32::Bk_Transparent);

		for (auto& guide : state.guides)
		{
			if (guide.horizontal)
			{
				Win32::MoveToEx(hdc, rc.left, guide.position + offset, nullptr);
				Win32::LineTo(hdc, rc.right, guide.position + offset);
			}
			else
			{
				Win32::MoveToEx(hdc, guide.position + offset, rc.top, nullptr);
				Win32::LineTo(hdc, guide.position + offset, rc.bottom);
			}
		}

		Win32::SetBkMode(hdc, oldMode);
		Win32::SelectObject(hdc, oldPen);
		Win32::DeleteObject(pen);
	}

	export void DrawTabOrderBadges(const DesignState& state, Win32::HDC hdc)
	{
		int offset = RulerOffset(state);
		constexpr int BADGE_SIZE = 20;

		auto badgeBrush = Win32::CreateSolidBrush(state.theme.selectionHighlight);
		auto badgePen = Win32::CreatePen(Win32::PenStyles::Solid, 1, state.theme.selectionHighlight);
		auto oldBrush = Win32::SelectObject(hdc, badgeBrush);
		auto oldPen = Win32::SelectObject(hdc, badgePen);
		auto oldTextColor = Win32::SetTextColor(hdc, Win32::MakeRgb(255, 255, 255));
		auto oldBkMode = Win32::SetBkMode(hdc, Win32::Bk_Transparent);

		auto font = Win32::CreateFontW(
			14, 0, 0, 0, 700, false, false, false, 1, 0, 0, 0, 0, L"Segoe UI");
		auto oldFont = Win32::SelectObject(hdc, font);

		for (int i = 0; i < static_cast<int>(state.entries.size()); ++i)
		{
			auto& ctrl = *state.entries[i].control;
			int bx = ctrl.rect.x + offset - BADGE_SIZE / 2;
			int by = ctrl.rect.y + offset - BADGE_SIZE / 2;

			Win32::Ellipse(hdc, bx, by, bx + BADGE_SIZE, by + BADGE_SIZE);

			auto label = std::to_wstring(ctrl.tabIndex);
			Win32::RECT textRc = { bx, by, bx + BADGE_SIZE, by + BADGE_SIZE };
			Win32::DrawTextW(hdc, label.c_str(), static_cast<int>(label.size()),
				&textRc, Win32::DrawTextFlags::Center | Win32::DrawTextFlags::VCenter |
				Win32::DrawTextFlags::SingleLine);
		}

		Win32::SelectObject(hdc, oldFont);
		Win32::DeleteObject(font);
		Win32::SetBkMode(hdc, oldBkMode);
		Win32::SetTextColor(hdc, oldTextColor);
		Win32::SelectObject(hdc, oldPen);
		Win32::SelectObject(hdc, oldBrush);
		Win32::DeleteObject(badgePen);
		Win32::DeleteObject(badgeBrush);
	}

}
