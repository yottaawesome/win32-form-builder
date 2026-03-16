#include <catch2/catch_test_macros.hpp>
import std;
import formbuilder;

using namespace FormDesigner;

TEST_CASE("EventMap stores and retrieves click handler", "[events]")
{
    EventMap events;
    bool called = false;
    events.onClick(1, [&](const ClickEvent&) { called = true; });

    auto handler = events.findClickHandler(1);
    REQUIRE(handler != nullptr);

    (*handler)(ClickEvent{ .controlId = 1, .controlHwnd = nullptr, .formHwnd = nullptr });
    REQUIRE(called);
}

TEST_CASE("EventMap returns nullptr for unregistered handler", "[events]")
{
    EventMap events;
    REQUIRE(events.findClickHandler(999) == nullptr);
    REQUIRE(events.findChangeHandler(999) == nullptr);
    REQUIRE(events.findDoubleClickHandler(999) == nullptr);
    REQUIRE(events.findSelectionChangeHandler(999) == nullptr);
    REQUIRE(events.findFocusHandler(999) == nullptr);
    REQUIRE(events.findBlurHandler(999) == nullptr);
    REQUIRE(events.findCheckHandler(999) == nullptr);
}

TEST_CASE("EventMap handles all 7 event types", "[events]")
{
    EventMap events;
    int callCount = 0;

    events.onClick(1, [&](const ClickEvent&) { callCount++; });
    events.onChange(2, [&](const ChangeEvent&) { callCount++; });
    events.onDoubleClick(3, [&](const DoubleClickEvent&) { callCount++; });
    events.onSelectionChange(4, [&](const SelectionChangeEvent&) { callCount++; });
    events.onFocus(5, [&](const FocusEvent&) { callCount++; });
    events.onBlur(6, [&](const BlurEvent&) { callCount++; });
    events.onCheck(7, [&](const CheckEvent&) { callCount++; });

    (*events.findClickHandler(1))(ClickEvent{});
    (*events.findChangeHandler(2))(ChangeEvent{});
    (*events.findDoubleClickHandler(3))(DoubleClickEvent{});
    (*events.findSelectionChangeHandler(4))(SelectionChangeEvent{});
    (*events.findFocusHandler(5))(FocusEvent{});
    (*events.findBlurHandler(6))(BlurEvent{});
    (*events.findCheckHandler(7))(CheckEvent{});

    REQUIRE(callCount == 7);
}

TEST_CASE("EventMap replaces handler for same ID", "[events]")
{
    EventMap events;
    int value = 0;

    events.onClick(1, [&](const ClickEvent&) { value = 1; });
    events.onClick(1, [&](const ClickEvent&) { value = 2; });

    (*events.findClickHandler(1))(ClickEvent{});
    REQUIRE(value == 2);
}

TEST_CASE("EventMap handles multiple IDs independently", "[events]")
{
    EventMap events;
    int value = 0;

    events.onClick(1, [&](const ClickEvent&) { value += 10; });
    events.onClick(2, [&](const ClickEvent&) { value += 20; });

    (*events.findClickHandler(1))(ClickEvent{});
    REQUIRE(value == 10);

    (*events.findClickHandler(2))(ClickEvent{});
    REQUIRE(value == 30);
}
