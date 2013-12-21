/*
 * Copyright (C) 2007 Staikos Computing Services Inc. <info@staikos.net>
 * Copyright (C) 2007 Apple Inc.  All rights reserved.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include "IntSize.h"
#include "LocalizedStrings.h"
#include "NotImplemented.h"
#include "PlatformString.h"

//#include <QCoreApplication>

namespace WebCore {

static String String_Translate(const char * s1, const char *sourceText, const char * s3)
{
    return String(sourceText);
}

String submitButtonDefaultLabel()
{
    return String_Translate("QWebPage", "Submit", "default label for Submit buttons in forms on web pages");
}

String inputElementAltText()
{
    return String_Translate("QWebPage", "Submit", "Submit (input element) alt text for <input> elements with no alt, title, or value");
}

String resetButtonDefaultLabel()
{
    return String_Translate("QWebPage", "Reset", "default label for Reset buttons in forms on web pages");
}

// in platform\win\Language.cpp
// String defaultLanguage()
// {
//     return "en";
// }

String searchableIndexIntroduction()
{
    return String_Translate("QWebPage", "This is a searchable index. Enter search keywords: ", "text that appears at the start of nearly-obsolete web pages in the form of a 'searchable index'");
}
    
String fileButtonChooseFileLabel()
{
    return String_Translate("QWebPage", "Choose File", "title for file button used in HTML forms");
}

String fileButtonNoFileSelectedLabel()
{
    return String_Translate("QWebPage", "No file selected", "text to display in file button used in HTML forms when no file is selected");
}

String contextMenuItemTagOpenLinkInNewWindow()
{
    return String_Translate("QWebPage", "Open in New Window", "Open in New Window context menu item");
}

String contextMenuItemTagDownloadLinkToDisk()
{
    return String_Translate("QWebPage", "Save Link...", "Download Linked File context menu item");
}

String contextMenuItemTagCopyLinkToClipboard()
{
    return String_Translate("QWebPage", "Copy Link", "Copy Link context menu item");
}

String contextMenuItemTagOpenImageInNewWindow()
{
    return String_Translate("QWebPage", "Open Image", "Open Image in New Window context menu item");
}

String contextMenuItemTagDownloadImageToDisk()
{
    return String_Translate("QWebPage", "Save Image", "Download Image context menu item");
}

String contextMenuItemTagCopyImageToClipboard()
{
    return String_Translate("QWebPage", "Copy Image", "Copy Link context menu item");
}

String contextMenuItemTagOpenFrameInNewWindow()
{
    return String_Translate("QWebPage", "Open Frame", "Open Frame in New Window context menu item");
}

String contextMenuItemTagCopy()
{
    return String_Translate("QWebPage", "Copy", "Copy context menu item");
}

String contextMenuItemTagGoBack()
{
    return String_Translate("QWebPage", "Go Back", "Back context menu item");
}

String contextMenuItemTagGoForward()
{
    return String_Translate("QWebPage", "Go Forward", "Forward context menu item");
}

String contextMenuItemTagStop()
{
    return String_Translate("QWebPage", "Stop", "Stop context menu item");
}

String contextMenuItemTagReload()
{
    return String_Translate("QWebPage", "Reload", "Reload context menu item");
}

String contextMenuItemTagCut()
{
    return String_Translate("QWebPage", "Cut", "Cut context menu item");
}

String contextMenuItemTagPaste()
{
    return String_Translate("QWebPage", "Paste", "Paste context menu item");
}

String contextMenuItemTagNoGuessesFound()
{
    return String_Translate("QWebPage", "No Guesses Found", "No Guesses Found context menu item");
}

String contextMenuItemTagIgnoreSpelling()
{
    return String_Translate("QWebPage", "Ignore", "Ignore Spelling context menu item");
}

String contextMenuItemTagLearnSpelling()
{
    return String_Translate("QWebPage", "Add To Dictionary", "Learn Spelling context menu item");
}

String contextMenuItemTagSearchWeb()
{
    return String_Translate("QWebPage", "Search The Web", "Search The Web context menu item");
}

String contextMenuItemTagLookUpInDictionary()
{
    return String_Translate("QWebPage", "Look Up In Dictionary", "Look Up in Dictionary context menu item");
}

String contextMenuItemTagOpenLink()
{
    return String_Translate("QWebPage", "Open Link", "Open Link context menu item");
}

String contextMenuItemTagIgnoreGrammar()
{
    return String_Translate("QWebPage", "Ignore", "Ignore Grammar context menu item");
}

String contextMenuItemTagSpellingMenu()
{
    return String_Translate("QWebPage", "Spelling", "Spelling and Grammar context sub-menu item");
}

String contextMenuItemTagShowSpellingPanel(bool show)
{
    return show ? String_Translate("QWebPage", "Show Spelling and Grammar", "menu item title") : 
                  String_Translate("QWebPage", "Hide Spelling and Grammar", "menu item title");
}

String contextMenuItemTagCheckSpelling()
{
    return String_Translate("QWebPage", "Check Spelling", "Check spelling context menu item");
}

String contextMenuItemTagCheckSpellingWhileTyping()
{
    return String_Translate("QWebPage", "Check Spelling While Typing", "Check spelling while typing context menu item");
}

String contextMenuItemTagCheckGrammarWithSpelling()
{
    return String_Translate("QWebPage", "Check Grammar With Spelling", "Check grammar with spelling context menu item");
}

String contextMenuItemTagFontMenu()
{
    return String_Translate("QWebPage", "Fonts", "Font context sub-menu item");
}

String contextMenuItemTagBold()
{
    return String_Translate("QWebPage", "Bold", "Bold context menu item");
}

String contextMenuItemTagItalic()
{
    return String_Translate("QWebPage", "Italic", "Italic context menu item");
}

String contextMenuItemTagUnderline()
{
    return String_Translate("QWebPage", "Underline", "Underline context menu item");
}

String contextMenuItemTagOutline()
{
    return String_Translate("QWebPage", "Outline", "Outline context menu item");
}

String contextMenuItemTagWritingDirectionMenu()
{
    return String_Translate("QWebPage", "Direction", "Writing direction context sub-menu item");
}

String contextMenuItemTagTextDirectionMenu()
{
    return String_Translate("QWebPage", "Text Direction", "Text direction context sub-menu item");
}

String contextMenuItemTagDefaultDirection()
{
    return String_Translate("QWebPage", "Default", "Default writing direction context menu item");
}

String contextMenuItemTagLeftToRight()
{
    return String_Translate("QWebPage", "LTR", "Left to Right context menu item");
}

String contextMenuItemTagRightToLeft()
{
    return String_Translate("QWebPage", "RTL", "Right to Left context menu item");
}

String contextMenuItemTagInspectElement()
{
    return String_Translate("QWebPage", "Inspect", "Inspect Element context menu item");
}

String searchMenuNoRecentSearchesText()
{
    return String_Translate("QWebPage", "No recent searches", "Label for only item in menu that appears when clicking on the search field image, when no searches have been performed");
}

String searchMenuRecentSearchesText()
{
    return String_Translate("QWebPage", "Recent searches", "label for first item in the menu that appears when clicking on the search field image, used as embedded menu title");
}

String searchMenuClearRecentSearchesText()
{
    return String_Translate("QWebPage", "Clear recent searches", "menu item in Recent Searches menu that empties menu's contents");
}

String AXWebAreaText()
{
    return String();
}

String AXLinkText()
{
    return String();
}

String AXListMarkerText()
{
    return String();
}

String AXImageMapText()
{
    return String();
}

String AXHeadingText()
{
    return String();
}

String AXDefinitionListTermText()
{
    return String();
}

String AXDefinitionListDefinitionText()
{
    return String();
}

String AXButtonActionVerb()
{
    return String();
}

String AXRadioButtonActionVerb()
{
    return String();
}

String AXTextFieldActionVerb()
{
    return String();
}

String AXCheckedCheckBoxActionVerb()
{
    return String();
}

String AXUncheckedCheckBoxActionVerb()
{
    return String();
}

String AXLinkActionVerb()
{
    return String();
}

String multipleFileUploadText(unsigned)
{
    return String();
}

String unknownFileSizeText()
{
    return String_Translate("QWebPage", "Unknown", "Unknown filesize FTP directory listing item");
}

String imageTitle(const String& filename, const IntSize& size)
{
    return String("pixels");
    //return String_Translate("QWebPage", "%1 (%2x%3 pixels)", "Title string for images").arg(filename).arg(size.width()).arg(size.height());
}

}
// vim: ts=4 sw=4 et
