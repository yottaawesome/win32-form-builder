export module designer:hit_testing;
import std;
import formbuilder;
import :state;

namespace Designer
{

	export auto IsSelected(const DesignState& state, int index) -> bool
	{
		return state.selection.contains(index);
	}

	export auto SingleSelection(const DesignState& state) -> int
	{
		if (state.selection.size() == 1)
			return *state.selection.begin();
		return -1;
	}

	export auto HitTest(const DesignState& state, int x, int y) -> int
	{
		for (int i = static_cast<int>(state.entries.size()) - 1; i >= 0; --i)
		{
			auto& r = state.entries[i].control->rect;
			if (x >= r.x && x < r.x + r.width &&
				y >= r.y && y < r.y + r.height)
				return i;
		}
		return -1;
	}

	export void GetHandleAnchors(const FormDesigner::Rect& r, Win32::POINT out[8])
	{
		int cx = r.x + r.width / 2;
		int cy = r.y + r.height / 2;
		int rx = r.x + r.width;
		int by = r.y + r.height;

		out[0] = { r.x - HANDLE_HALF, r.y - HANDLE_HALF };
		out[1] = { cx  - HANDLE_HALF, r.y - HANDLE_HALF };
		out[2] = { rx  - HANDLE_HALF, r.y - HANDLE_HALF };
		out[3] = { r.x - HANDLE_HALF, cy  - HANDLE_HALF };
		out[4] = { rx  - HANDLE_HALF, cy  - HANDLE_HALF };
		out[5] = { r.x - HANDLE_HALF, by  - HANDLE_HALF };
		out[6] = { cx  - HANDLE_HALF, by  - HANDLE_HALF };
		out[7] = { rx  - HANDLE_HALF, by  - HANDLE_HALF };
	}

	export auto HitTestHandle(const DesignState& state, int x, int y) -> int
	{
		int sel = SingleSelection(state);
		if (sel < 0 || sel >= static_cast<int>(state.entries.size()))
			return -1;

		Win32::POINT anchors[8];
		GetHandleAnchors(state.entries[sel].control->rect, anchors);

		for (int i = 0; i < 8; ++i)
		{
			if (x >= anchors[i].x && x < anchors[i].x + HANDLE_SIZE &&
				y >= anchors[i].y && y < anchors[i].y + HANDLE_SIZE)
				return i;
		}
		return -1;
	}

	export auto CursorForHandle(int handle) -> Win32::LPCWSTR
	{
		switch (handle)
		{
		case 0: case 7: return Win32::Cursors::SizeNWSE;
		case 2: case 5: return Win32::Cursors::SizeNESW;
		case 1: case 6: return Win32::Cursors::SizeNS;
		case 3: case 4: return Win32::Cursors::SizeWE;
		default:         return Win32::Cursors::Arrow;
		}
	}

	constexpr int FORM_EDGE_THRESHOLD = 5;

	export auto HitTestFormBoundary(const DesignState& state, int x, int y) -> FormEdge
	{
		int fw = state.form.width;
		int fh = state.form.height;
		bool nearRight  = std::abs(x - fw) <= FORM_EDGE_THRESHOLD && y >= -FORM_EDGE_THRESHOLD && y <= fh + FORM_EDGE_THRESHOLD;
		bool nearBottom = std::abs(y - fh) <= FORM_EDGE_THRESHOLD && x >= -FORM_EDGE_THRESHOLD && x <= fw + FORM_EDGE_THRESHOLD;

		if (nearRight && nearBottom) return FormEdge::BottomRight;
		if (nearRight)              return FormEdge::Right;
		if (nearBottom)             return FormEdge::Bottom;
		return FormEdge::None;
	}

	export auto CursorForFormEdge(FormEdge edge) -> Win32::LPCWSTR
	{
		switch (edge)
		{
		case FormEdge::Right:       return Win32::Cursors::SizeWE;
		case FormEdge::Bottom:      return Win32::Cursors::SizeNS;
		case FormEdge::BottomRight: return Win32::Cursors::SizeNWSE;
		default:                    return Win32::Cursors::Arrow;
		}
	}

	export void ApplyResize(FormDesigner::Rect& r, int handle, int dx, int dy,
		const Win32::POINT& startPos, const Win32::SIZE& startSize)
	{
		bool moveLeft   = (handle == 0 || handle == 3 || handle == 5);
		bool moveTop    = (handle == 0 || handle == 1 || handle == 2);
		bool moveRight  = (handle == 2 || handle == 4 || handle == 7);
		bool moveBottom = (handle == 5 || handle == 6 || handle == 7);

		int newX = startPos.x;
		int newY = startPos.y;
		int newW = startSize.cx;
		int newH = startSize.cy;

		if (moveLeft)   { newX += dx; newW -= dx; }
		if (moveTop)    { newY += dy; newH -= dy; }
		if (moveRight)  { newW += dx; }
		if (moveBottom) { newH += dy; }

		if (newW < MIN_CONTROL_SIZE)
		{
			if (moveLeft) newX -= (MIN_CONTROL_SIZE - newW);
			newW = MIN_CONTROL_SIZE;
		}
		if (newH < MIN_CONTROL_SIZE)
		{
			if (moveTop) newY -= (MIN_CONTROL_SIZE - newH);
			newH = MIN_CONTROL_SIZE;
		}

		r.x = newX;
		r.y = newY;
		r.width = newW;
		r.height = newH;
	}

	export auto HitTestUserGuide(const DesignState& state, int formX, int formY) -> int
	{
		for (int i = 0; i < static_cast<int>(state.userGuides.size()); ++i)
		{
			auto& g = state.userGuides[i];
			int coord = g.horizontal ? formY : formX;
			if (std::abs(coord - g.position) <= 3)
				return i;
		}
		return -1;
	}

}
