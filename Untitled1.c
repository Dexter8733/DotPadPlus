#include <windows.h>
#include <commdlg.h>
#include <stdio.h>

#define ID_EDIT 1
#define IDM_NEW 101
#define IDM_OPEN 102
#define IDM_SAVE 103
#define IDM_EXIT 104
#define IDM_UNDO 201
#define IDM_CUT 202
#define IDM_COPY 203
#define IDM_PASTE 204
#define IDM_SELECTALL 205
#define IDM_WRAP 301

HWND hEdit;
BOOL wrap=TRUE;
char currentFile[MAX_PATH]="";

void LoadFile(const char *path){
    FILE *f=fopen(path,"rb"); if(!f)return;
    fseek(f,0,SEEK_END); long sz=ftell(f); fseek(f,0,SEEK_SET);
    char *buf=(char*)malloc(sz+1); if(!buf){fclose(f); return;}
    fread(buf,1,sz,f); buf[sz]=0; SetWindowTextA(hEdit,buf);
    free(buf); fclose(f); lstrcpyA(currentFile,path);
}

void SaveFile(const char *path){
    int len=GetWindowTextLengthA(hEdit);
    char *buf=(char*)malloc(len+1); if(!buf)return;
    GetWindowTextA(hEdit,buf,len+1);
    FILE *f=fopen(path,"wb"); if(f){fwrite(buf,1,len,f); fclose(f); lstrcpyA(currentFile,path);}
    free(buf);
}

void DoOpen(HWND hwnd){
    OPENFILENAMEA ofn={0}; char file[MAX_PATH]="";
    ofn.lStructSize=sizeof(ofn); ofn.hwndOwner=hwnd; ofn.lpstrFile=file;
    ofn.nMaxFile=MAX_PATH; ofn.lpstrFilter="Text Files\0*.txt\0All Files\0*.*\0";
    if(GetOpenFileNameA(&ofn)) LoadFile(file);
}

void DoSave(HWND hwnd){
    if(currentFile[0]){SaveFile(currentFile); return;}
    OPENFILENAMEA ofn={0}; char file[MAX_PATH]="";
    ofn.lStructSize=sizeof(ofn); ofn.hwndOwner=hwnd; ofn.lpstrFile=file;
    ofn.nMaxFile=MAX_PATH; ofn.lpstrFilter="Text Files\0*.txt\0All Files\0*.*\0";
    ofn.Flags=OFN_OVERWRITEPROMPT;
    if(GetSaveFileNameA(&ofn)) SaveFile(file);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
    switch(msg){
    case WM_CREATE:{
        HMENU bar=CreateMenu(),f=CreatePopupMenu(),e=CreatePopupMenu(),fm=CreatePopupMenu();
        AppendMenu(f,MF_STRING,IDM_NEW,"New"); AppendMenu(f,MF_STRING,IDM_OPEN,"Open");
        AppendMenu(f,MF_STRING,IDM_SAVE,"Save"); AppendMenu(f,MF_STRING,IDM_EXIT,"Exit");
        AppendMenu(e,MF_STRING,IDM_UNDO,"Undo"); AppendMenu(e,MF_STRING,IDM_CUT,"Cut");
        AppendMenu(e,MF_STRING,IDM_COPY,"Copy"); AppendMenu(e,MF_STRING,IDM_PASTE,"Paste");
        AppendMenu(e,MF_STRING,IDM_SELECTALL,"Select All");
        AppendMenu(fm,MF_STRING|MF_CHECKED,IDM_WRAP,"Word Wrap");
        AppendMenu(bar,MF_POPUP,(UINT_PTR)f,"File");
        AppendMenu(bar,MF_POPUP,(UINT_PTR)e,"Edit");
        AppendMenu(bar,MF_POPUP,(UINT_PTR)fm,"Format");
        SetMenu(hwnd,bar);
        RECT r; GetClientRect(hwnd,&r);
        hEdit=CreateWindowEx(0,"EDIT","",WS_CHILD|WS_VISIBLE|WS_VSCROLL|
            ES_LEFT|ES_MULTILINE|ES_AUTOVSCROLL,0,0,r.right,r.bottom,hwnd,(HMENU)ID_EDIT,NULL,NULL);
        break;
    }
    case WM_SIZE: MoveWindow(hEdit,0,0,LOWORD(lParam),HIWORD(lParam),TRUE); break;
    case WM_COMMAND: switch(LOWORD(wParam)){
        case IDM_EXIT: PostQuitMessage(0); break;
        case IDM_UNDO: SendMessage(hEdit,EM_UNDO,0,0); break;
        case IDM_CUT: SendMessage(hEdit,WM_CUT,0,0); break;
        case IDM_COPY: SendMessage(hEdit,WM_COPY,0,0); break;
        case IDM_PASTE: SendMessage(hEdit,WM_PASTE,0,0); break;
        case IDM_SELECTALL: SendMessage(hEdit,EM_SETSEL,0,-1); break;
        case IDM_WRAP:{
            wrap=!wrap;
            HMENU m=GetMenu(hwnd);
            CheckMenuItem(m,IDM_WRAP,MF_BYCOMMAND|(wrap?MF_CHECKED:MF_UNCHECKED));
            LONG s=GetWindowLong(hEdit,GWL_STYLE);
            if(wrap)s&=~WS_HSCROLL; else s|=WS_HSCROLL;
            SetWindowLong(hEdit,GWL_STYLE,s);
            SetWindowPos(hEdit,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_FRAMECHANGED);
            break;
        }
        case IDM_NEW: SetWindowTextA(hEdit,""); currentFile[0]=0; break;
        case IDM_OPEN: DoOpen(hwnd); break;
        case IDM_SAVE: DoSave(hwnd); break;
    } break;
    case WM_DESTROY: PostQuitMessage(0); break;
    default: return DefWindowProc(hwnd,msg,wParam,lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE h,HINSTANCE p,LPSTR c,int n){
    WNDCLASS wc={0}; wc.lpfnWndProc=WndProc; wc.hInstance=h;
    wc.lpszClassName="PlainTextEditor"; wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
    wc.hCursor=LoadCursor(NULL,IDC_IBEAM); RegisterClass(&wc);
    HWND w=CreateWindow("PlainTextEditor","Plain Text Editor",
        WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,800,600,
        NULL,NULL,h,NULL);
    ShowWindow(w,n); UpdateWindow(w);
    MSG msg;
    while(GetMessage(&msg,NULL,0,0)){TranslateMessage(&msg);DispatchMessage(&msg);}
    return msg.wParam;
}
