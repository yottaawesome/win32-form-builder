export module designer:alignment;
import std;
import formbuilder;

namespace Designer
{

	// Collects non-locked control pointers from the given indices.
	export auto CollectUnlocked(std::vector<FormDesigner::Control>& controls,
		const std::set<int>& selection) -> std::vector<FormDesigner::Control*>
	{
		auto result = std::vector<FormDesigner::Control*>{};
		for (int idx : selection)
			if (idx >= 0 && idx < static_cast<int>(controls.size()) && !controls[idx].locked)
				result.push_back(&controls[idx]);
		return result;
	}

	// --- Alignment (2+ controls) ---

	export void AlignLeft(std::vector<FormDesigner::Control*>& ctrls)
	{
		if (ctrls.size() < 2) return;
		int minX = ctrls[0]->rect.x;
		for (auto* c : ctrls) minX = (std::min)(minX, c->rect.x);
		for (auto* c : ctrls) c->rect.x = minX;
	}

	export void AlignCenterH(std::vector<FormDesigner::Control*>& ctrls)
	{
		if (ctrls.size() < 2) return;
		int minX = ctrls[0]->rect.x, maxRight = ctrls[0]->rect.x + ctrls[0]->rect.width;
		for (auto* c : ctrls)
		{
			minX = (std::min)(minX, c->rect.x);
			maxRight = (std::max)(maxRight, c->rect.x + c->rect.width);
		}
		int center = (minX + maxRight) / 2;
		for (auto* c : ctrls) c->rect.x = center - c->rect.width / 2;
	}

	export void AlignRight(std::vector<FormDesigner::Control*>& ctrls)
	{
		if (ctrls.size() < 2) return;
		int maxRight = ctrls[0]->rect.x + ctrls[0]->rect.width;
		for (auto* c : ctrls) maxRight = (std::max)(maxRight, c->rect.x + c->rect.width);
		for (auto* c : ctrls) c->rect.x = maxRight - c->rect.width;
	}

	export void AlignTop(std::vector<FormDesigner::Control*>& ctrls)
	{
		if (ctrls.size() < 2) return;
		int minY = ctrls[0]->rect.y;
		for (auto* c : ctrls) minY = (std::min)(minY, c->rect.y);
		for (auto* c : ctrls) c->rect.y = minY;
	}

	export void AlignMiddleV(std::vector<FormDesigner::Control*>& ctrls)
	{
		if (ctrls.size() < 2) return;
		int minY = ctrls[0]->rect.y, maxBottom = ctrls[0]->rect.y + ctrls[0]->rect.height;
		for (auto* c : ctrls)
		{
			minY = (std::min)(minY, c->rect.y);
			maxBottom = (std::max)(maxBottom, c->rect.y + c->rect.height);
		}
		int middle = (minY + maxBottom) / 2;
		for (auto* c : ctrls) c->rect.y = middle - c->rect.height / 2;
	}

	export void AlignBottom(std::vector<FormDesigner::Control*>& ctrls)
	{
		if (ctrls.size() < 2) return;
		int maxBottom = ctrls[0]->rect.y + ctrls[0]->rect.height;
		for (auto* c : ctrls) maxBottom = (std::max)(maxBottom, c->rect.y + c->rect.height);
		for (auto* c : ctrls) c->rect.y = maxBottom - c->rect.height;
	}

	// --- Distribution (3+ controls) ---

	export void DistributeHorizontally(std::vector<FormDesigner::Control*>& ctrls)
	{
		if (ctrls.size() < 3) return;
		std::ranges::sort(ctrls, {}, [](auto* c) { return c->rect.x; });

		int totalWidth = 0;
		for (auto* c : ctrls) totalWidth += c->rect.width;

		int minX = ctrls.front()->rect.x;
		int maxRight = ctrls.back()->rect.x + ctrls.back()->rect.width;
		int totalSpace = maxRight - minX - totalWidth;
		int gaps = static_cast<int>(ctrls.size()) - 1;
		double spacing = static_cast<double>(totalSpace) / gaps;

		double x = minX;
		for (auto* c : ctrls)
		{
			c->rect.x = static_cast<int>(x + 0.5);
			x += c->rect.width + spacing;
		}
	}

	export void DistributeVertically(std::vector<FormDesigner::Control*>& ctrls)
	{
		if (ctrls.size() < 3) return;
		std::ranges::sort(ctrls, {}, [](auto* c) { return c->rect.y; });

		int totalHeight = 0;
		for (auto* c : ctrls) totalHeight += c->rect.height;

		int minY = ctrls.front()->rect.y;
		int maxBottom = ctrls.back()->rect.y + ctrls.back()->rect.height;
		int totalSpace = maxBottom - minY - totalHeight;
		int gaps = static_cast<int>(ctrls.size()) - 1;
		double spacing = static_cast<double>(totalSpace) / gaps;

		double y = minY;
		for (auto* c : ctrls)
		{
			c->rect.y = static_cast<int>(y + 0.5);
			y += c->rect.height + spacing;
		}
	}

	// --- Sizing (2+ controls) ---

	export void MakeSameWidth(std::vector<FormDesigner::Control*>& ctrls)
	{
		if (ctrls.size() < 2) return;
		int maxW = 0;
		for (auto* c : ctrls) maxW = (std::max)(maxW, c->rect.width);
		for (auto* c : ctrls) c->rect.width = maxW;
	}

	export void MakeSameHeight(std::vector<FormDesigner::Control*>& ctrls)
	{
		if (ctrls.size() < 2) return;
		int maxH = 0;
		for (auto* c : ctrls) maxH = (std::max)(maxH, c->rect.height);
		for (auto* c : ctrls) c->rect.height = maxH;
	}

	export void MakeSameSize(std::vector<FormDesigner::Control*>& ctrls)
	{
		if (ctrls.size() < 2) return;
		int maxW = 0, maxH = 0;
		for (auto* c : ctrls)
		{
			maxW = (std::max)(maxW, c->rect.width);
			maxH = (std::max)(maxH, c->rect.height);
		}
		for (auto* c : ctrls)
		{
			c->rect.width = maxW;
			c->rect.height = maxH;
		}
	}

} // namespace Designer
