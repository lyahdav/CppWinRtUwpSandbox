//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "pch.h"
#include "MainPage.h"
#include "MainPage.g.cpp"

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Automation::Peers;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Interop;
using namespace winrt::Windows::UI::Xaml::Media;
using namespace winrt::Windows::UI::Xaml::Navigation;

namespace winrt {
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;
    using namespace Windows::UI;
    using namespace winrt::Windows::UI::Xaml::Controls;
    using namespace winrt::Windows::UI::Xaml;
    using namespace winrt::Windows::UI::Xaml::Controls::Primitives;
}

namespace winrt::SDKTemplate::implementation
{
    SDKTemplate::MainPage MainPage::current{ nullptr };

    /*static*/ void FixMenuFlyoutIconsIfDarkTheme(
        const winrt::MenuFlyout& menuFlyout) noexcept {
        // Workaround Xaml bug with MenuFlyoutItem icons and dark mode:
        // https://github.com/microsoft/microsoft-ui-xaml/issues/5381

        menuFlyout.Opened([](winrt::IInspectable const& sender, const auto&) {
            FixMenuItemIconsRecursively(sender.as<winrt::MenuFlyout>().Items());
            });
    }

    /*static*/ void FixMenuItemIconsRecursively(
        const winrt::IVector<winrt::MenuFlyoutItemBase>& items) noexcept {
        for (auto const& item : items) {
            if (auto menuFlyoutItem = item.try_as<winrt::MenuFlyoutItem>()) {
                if (menuFlyoutItem.ActualTheme() == winrt::ElementTheme::Dark) {
                    menuFlyoutItem.Icon().Foreground(nullptr);
                }
            }
            else if (auto menuFlyoutSubItem = item.try_as<winrt::MenuFlyoutSubItem>()) {
                if (menuFlyoutSubItem.ActualTheme() == winrt::ElementTheme::Dark) {
                    menuFlyoutSubItem.Icon().Foreground(nullptr);
                }
                FixMenuItemIconsRecursively(menuFlyoutSubItem.Items());
            }
        }
    }

    winrt::Popup GetPopup() {
        auto popups =
            VisualTreeHelper::GetOpenPopups(Window::Current());
        if (popups.Size() > 0)
            return popups.GetAt(0);
        return nullptr;
    }

    MainPage::MainPage()
    {
        InitializeComponent();
        SampleTitle().Text(FEATURE_NAME());

        // This is a static public property that allows downstream pages to get a handle to the MainPage instance
        // in order to call methods that are in this class.
        MainPage::current = *this;

        StackPanel stackPanel;

        auto makeButtonWithContentDialog = [](bool setBgColor, bool updatePopupTheme) {
            Button button;
            std::wostringstream wostringstream;
            wostringstream << L"Open ContentDialog, setBgColor = " << setBgColor << L", updatePopupTheme = " << updatePopupTheme;
            button.Content(winrt::box_value(wostringstream.str()));

            button.Click([=](auto const &...) {
                ContentDialog dialog{};
                if (setBgColor) {
                    dialog.Background(winrt::Media::SolidColorBrush(winrt::Colors::Black()));
                }
                dialog.Title(winrt::box_value(L"Title"));
                dialog.Content(winrt::box_value(L"Content"));
                dialog.XamlRoot(button.XamlRoot());
                dialog.CloseButtonText(L"Ok");
                if (updatePopupTheme) {
                    dialog.Opened([=](auto const &...) {
                        if (auto popup = GetPopup()) {
                            popup.RequestedTheme(button.ActualTheme());
                        }
                        });
                }
                dialog.ShowAsync();
                });
            return button;
        };
        for (auto updatePopupTheme : { false, true }) {
            for (auto setBgColor : { false, true }) {
                stackPanel.Children().Append(makeButtonWithContentDialog(setBgColor, updatePopupTheme));
            }
        }

        // For dark mode:
        stackPanel.RequestedTheme(Windows::UI::Xaml::ElementTheme::Dark);
        stackPanel.Background(winrt::Media::SolidColorBrush(winrt::Colors::Black()));

        this->Content(stackPanel);
    }

    void MainPage::OnNavigatedTo(NavigationEventArgs const&)
    {
        // Populate the ListBox with the scenarios as defined in SampleConfiguration.cpp.
        auto itemCollection = single_threaded_observable_vector<IInspectable>();
        int i = 1;
        for (auto s : MainPage::scenarios())
        {
            s.Title = to_hstring(i++) + L") " + s.Title;
            itemCollection.Append(box_value(s));
        }

        // Set the newly created itemCollection as the ListBox ItemSource.
        ScenarioControl().ItemsSource(itemCollection);

        int startingScenarioIndex;

        if (Window::Current().Bounds().Width < 640)
        {
            startingScenarioIndex = -1;
        }
        else
        {
            startingScenarioIndex = 0;
        }

        ScenarioControl().SelectedIndex(startingScenarioIndex);
        ScenarioControl().ScrollIntoView(ScenarioControl().SelectedItem());
    }

    void MainPage::Navigate(TypeName const& typeName, IInspectable const& parameter)
    {
        int index;
        for (index = static_cast<int>(scenarios().Size()) - 1; index >= 0; --index)
        {
            if (scenarios().GetAt(index).ClassName == typeName)
            {
                break;
            }
        }
        navigating = true;
        ScenarioControl().SelectedIndex(index);
        navigating = false;

        NavigateTo(typeName, parameter);
    }

    void MainPage::NavigateTo(TypeName const& typeName, IInspectable const& parameter)
    {
        // Clear the status block when changing scenarios
        NotifyUser(hstring(), NotifyType::StatusMessage);

        // Navigate to the selected scenario.
        ScenarioFrame().Navigate(typeName, parameter);

        if (Window::Current().Bounds().Width < 640)
        {
            Splitter().IsPaneOpen(false);
        }
    }

    void MainPage::ScenarioControl_SelectionChanged(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs const&)
    {
        ListBox scenarioListBox = sender.as<ListBox>();
        IInspectable selectedItem = scenarioListBox.SelectedItem();
        if (selectedItem && !navigating)
        {
            Scenario s = unbox_value<Scenario>(selectedItem);
            NavigateTo(s.ClassName, IInspectable{ nullptr });
        }
    }

    void MainPage::Footer_Click(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const&)
    {        
        Uri uri{ unbox_value<hstring>(sender.as<HyperlinkButton>().Tag()) };
        Windows::System::Launcher::LaunchUriAsync(uri);
    }

    void MainPage::Button_Click(Windows::Foundation::IInspectable const&, Windows::UI::Xaml::RoutedEventArgs const&)
    {
        Splitter().IsPaneOpen(!Splitter().IsPaneOpen());
    }

    void MainPage::NotifyUser(hstring const& strMessage, NotifyType type)
    {
        if (Dispatcher().HasThreadAccess())
        {
            UpdateStatus(strMessage, type);
        }
        else
        {
            Dispatcher().RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, [strMessage, type, this]()
            {
                UpdateStatus(strMessage, type);
            });
        }
    }

    void MainPage::UpdateStatus(const hstring& strMessage, NotifyType type)
    {
        switch (type)
        {
        case NotifyType::StatusMessage:
            StatusBorder().Background(SolidColorBrush(Windows::UI::Colors::Green()));
            break;
        case NotifyType::ErrorMessage:
            StatusBorder().Background(SolidColorBrush(Windows::UI::Colors::Red()));
            break;
        default:
            break;
        }

        StatusBlock().Text(strMessage);

        // Collapse the StatusBlock if it has no text to conserve real estate.
        if (!strMessage.empty())
        {
            StatusBorder().Visibility(Windows::UI::Xaml::Visibility::Visible);
            StatusPanel().Visibility(Windows::UI::Xaml::Visibility::Visible);
        }
        else
        {
            StatusBorder().Visibility(Windows::UI::Xaml::Visibility::Collapsed);
            StatusPanel().Visibility(Windows::UI::Xaml::Visibility::Collapsed);
        }

        // Raise an event if necessary to enable a screen reader to announce the status update.
        auto peer = FrameworkElementAutomationPeer::FromElement(StatusBlock()).as<FrameworkElementAutomationPeer>();
        if (peer != nullptr)
        {
            peer.RaiseAutomationEvent(AutomationEvents::LiveRegionChanged);
        }
    }
}
