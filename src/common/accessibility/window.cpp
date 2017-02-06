#include "window.h"
#include "element.h"
#include "application.h"

#define internal static

/*
 * NOTE(koekeishiya): The following files must also be linked against:
 *
 * common/accessibility/element.cpp
 *
 * */

internal inline void
AddFlags(macos_window *Window, uint32_t Flag)
{
    Window->Flags |= Flag;
}

// NOTE(koekeishiya): Caller is responsible for calling 'AXLibDestroyWindow()'. */
macos_window *AXLibConstructWindow(macos_application *Application, AXUIElementRef WindowRef)
{
    macos_window *Window = (macos_window *) malloc(sizeof(macos_window));
    memset(Window, 0, sizeof(macos_window));

    Window->Ref = (AXUIElementRef) CFRetain(WindowRef);
    AXLibGetWindowRole(Window->Ref, &Window->Mainrole);
    AXLibGetWindowSubrole(Window->Ref, &Window->Subrole);

    Window->Owner = Application;
    Window->Id = AXLibGetWindowID(Window->Ref);
    Window->Name = AXLibGetWindowTitle(Window->Ref);

    Window->Position = AXLibGetWindowPosition(Window->Ref);
    Window->Size = AXLibGetWindowSize(Window->Ref);

    if(AXLibIsWindowMovable(Window->Ref))
        AddFlags(Window, Window_Movable);

    if(AXLibIsWindowResizable(Window->Ref))
        AddFlags(Window, Window_Resizable);

    if(AXLibIsWindowMinimized(Window->Ref))
        AddFlags(Window, Window_Minimized);

    return Window;
}

macos_window *AXLibCopyWindow(macos_window *Window)
{
    macos_window *Result = (macos_window *) malloc(sizeof(macos_window));
    memset(Result, 0, sizeof(macos_window));

    Result->Ref = (AXUIElementRef) CFRetain(Window->Ref);

    if(Window->Mainrole)
        Result->Mainrole = (AXUIElementRef) CFRetain(Window->Mainrole);

    if(Window->Subrole)
        Result->Subrole = (AXUIElementRef) CFRetain(Window->Subrole);

    Result->Owner = Window->Owner;
    Result->Id = Window->Id;
    Result->Name = strdup(Window->Name);

    Result->Position = Window->Position;
    Result->Size = Window->Size;

    Result->Flags = Window->Flags;

    return Result;
}

/* NOTE(koekeishiya): The caller is responsible for passing a valid window! */
bool AXLibIsWindowStandard(macos_window *Window)
{
    bool Result = ((Window->Mainrole && CFEqual(Window->Mainrole, kAXWindowRole)) &&
                   (Window->Subrole && CFEqual(Window->Subrole, kAXStandardWindowSubrole)));
    return Result;
}

/* NOTE(koekeishiya): The caller is responsible for passing a valid window! */
bool AXLibWindowHasRole(macos_window *Window, CFTypeRef Role)
{
    bool Result = ((Window->Mainrole && CFEqual(Window->Mainrole, Role)) ||
                   (Window->Subrole && CFEqual(Window->Subrole, Role)));
    return Result;
}

/* NOTE(koekeishiya): This is not yet a thing.
bool AXLibIsWindowCustom(macos_window *Window)
{
    bool Result = ((Window->Customrole) &&
                   ((Window->Mainrole && CFEqual(Window->Mainrole, Window->Customrole)) ||
                    (Window->Subrole && CFEqual(Window->Subrole, Window->Customrole))));
    return Result;
}

bool AXLibWindowHasCustomRole(macos_window *Window, CFTypeRef Role)
{
    bool Result = ((Window->Type.CustomRole) &&
                   (CFEqual(Role, Window->Type.CustomRole)));
    return Result;
}

*/

// NOTE(koekeishiya): Caller is responsible for memory.
macos_window **AXLibWindowListForApplication(macos_application *Application)
{
    macos_window **WindowList = NULL;

    CFArrayRef Windows = (CFArrayRef) AXLibGetWindowProperty(Application->Ref, kAXWindowsAttribute);
    if(Windows)
    {
        CFIndex Count = CFArrayGetCount(Windows);
        WindowList = (macos_window **) malloc((Count + 1) * sizeof(macos_window *));
        WindowList[Count] = NULL;

        for(CFIndex Index = 0; Index < Count; ++Index)
        {
            AXUIElementRef Ref = (AXUIElementRef) CFArrayGetValueAtIndex(Windows, Index);
            macos_window *Window = AXLibConstructWindow(Application, Ref);
            WindowList[Index] = Window;
        }

        CFRelease(Windows);
    }

    return WindowList;
}

/* NOTE(koekeishiya): The caller is responsible for passing a valid window! */
void AXLibDestroyWindow(macos_window *Window)
{
    if(Window->Mainrole)
        CFRelease(Window->Mainrole);

    if(Window->Subrole)
        CFRelease(Window->Subrole);

    if(Window->Name)
        free(Window->Name);

    CFRelease(Window->Ref);
    free(Window);
}
