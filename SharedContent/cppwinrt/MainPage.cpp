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

    MainPage::MainPage()
    {
        InitializeComponent();
        SampleTitle().Text(FEATURE_NAME());

        // This is a static public property that allows downstream pages to get a handle to the MainPage instance
        // in order to call methods that are in this class.
        MainPage::current = *this;

        StackPanel stackPanel;
        StackPanel stackPanelLight;
        StackPanel stackPanelDark;

        stackPanelDark.RequestedTheme(ElementTheme::Dark);
        stackPanelDark.Background(SolidColorBrush(Windows::UI::Colors::Black()));

        CommandBarFlyout cbf;
        AppBarButton abb;
        abb.Label(L"Button");
        BitmapIcon icon;
        std::wstring uriFile{ L"ms-appx:///Assets/app-facebook.png" };
        Windows::Foundation::Uri uri{ uriFile };
        icon.UriSource(uri);
        abb.Icon(icon);
        cbf.SecondaryCommands().Append(abb);

        Button button1;
        button1.Content(winrt::box_value(L"CommandBarFlyout"));
        button1.Flyout(cbf);
        stackPanelLight.Children().Append(button1);

        Button button2;
        button2.Content(winrt::box_value(L"CommandBarFlyout"));
        button2.Flyout(cbf);
        stackPanelDark.Children().Append(button2);

        auto makeMenuFlyout = [=]() {
            MenuFlyout mf;

            MenuFlyoutItem mfi2;
            mfi2.Text(L"Menu Item 1");
            BitmapIcon icon2;
            icon2.UriSource(uri);
            mfi2.Icon(icon2);
            mf.Items().Append(mfi2);

            MenuFlyoutItem mfi3;
            mfi3.Text(L"Menu Item 2");
            BitmapIcon icon3;
            icon3.UriSource(uri);
            mfi3.Icon(icon3);
            mf.Items().Append(mfi3);

            return mf;
        };

        auto AddMenuFlyoutButtonsToPanel = [=](const winrt::StackPanel &stackPanel, bool includeFix) {
            auto mf1 = makeMenuFlyout();
            if (includeFix) {
                FixMenuFlyoutIconsIfDarkTheme(mf1);
            }
            Button button3;
            button3.Content(winrt::box_value(includeFix ? L"MenuFlyout using ShowAt with fix" : L"MenuFlyout using ShowAt without fix"));
            button3.Click([=](const auto &...) {
                const auto options = winrt::FlyoutShowOptions();
                options.Placement(winrt::FlyoutPlacementMode::Top);
                mf1.ShowAt(button3, options);
                });

            auto mf2 = makeMenuFlyout();
            if (includeFix) {
                FixMenuFlyoutIconsIfDarkTheme(mf2);
            }
            Button buttonWithFlyoutSet;
            buttonWithFlyoutSet.Content(winrt::box_value(includeFix ? L"MenuFlyout using Flyout property with fix" : L"MenuFlyout using Flyout property without fix"));
            buttonWithFlyoutSet.Flyout(mf2);

            stackPanel.Children().Append(button3);
            stackPanel.Children().Append(buttonWithFlyoutSet);
        };

        AddMenuFlyoutButtonsToPanel(stackPanelDark, false);
        AddMenuFlyoutButtonsToPanel(stackPanelDark, true);
        AddMenuFlyoutButtonsToPanel(stackPanelLight, false);
        AddMenuFlyoutButtonsToPanel(stackPanelLight, true);


        stackPanel.Children().Append(stackPanelLight);
        stackPanel.Children().Append(stackPanelDark);

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
