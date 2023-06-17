#include <stdio.h>
#include <pthread.h>
#include <windows.h>

#define NUM_THREADS 4  // Número de threads a serem criadas
#define NUM_LINES 50   // Número de linhas a serem escritas por thread

// Estrutura de dados para os argumentos da thread
typedef struct {
    int thread_id;
    HWND hwnd;
    HWND text_hwnd;
} ThreadArgs;

// Procedimento de janela para as janelas das threads
LRESULT CALLBACK ThreadWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// Função para criar e personalizar uma janela da thread
void createThreadWindow(ThreadArgs* thread_args, int thread_id, int x, int y, int width, int height)
{
    // Criar janela para a thread
    wchar_t window_title[20];
    swprintf_s(window_title, sizeof(window_title) / sizeof(window_title[0]), L"Thread %d", thread_id);

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = ThreadWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = window_title;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);  // Fundo preto
    RegisterClass(&wc);

    // Criar a janela da thread
    thread_args->hwnd = CreateWindowExW(0, wc.lpszClassName, window_title, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        x, y, width, height,
        NULL, NULL, wc.hInstance, NULL);

    // Criar controle de edição de texto
    thread_args->text_hwnd = CreateWindowW(L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
        0, 0, width, height,
        thread_args->hwnd, NULL, wc.hInstance, NULL);

    // Adicionar barra de rolagem ao controle de texto
    SetWindowLongPtr(thread_args->text_hwnd, GWL_STYLE,
        GetWindowLongPtr(thread_args->text_hwnd, GWL_STYLE) | WS_VSCROLL);

    // Mostrar a janela
    ShowWindow(thread_args->hwnd, SW_SHOW);

    // Ajustar o tamanho do controle de texto para incluir a barra de rolagem
    RECT client_rect;
    GetClientRect(thread_args->hwnd, &client_rect);
    int text_width = client_rect.right - client_rect.left;
    int text_height = client_rect.bottom - client_rect.top;
    int scrollbar_width = GetSystemMetrics(SM_CXVSCROLL);
    SetWindowPos(thread_args->text_hwnd, NULL, 0, 0, text_width - scrollbar_width, text_height, SWP_NOMOVE | SWP_NOZORDER);
}

// Função para escrever uma linha na janela da thread
void writeLine(HWND text_hwnd, const char* line)
{
    wchar_t wline[100];
    swprintf_s(wline, sizeof(wline) / sizeof(wline[0]), L"%hs\n", line);

    // Inserir linha na janela
    int text_length = GetWindowTextLengthW(text_hwnd);
    SendMessageW(text_hwnd, EM_SETSEL, text_length, text_length);  // Selecionar o final do texto
    SendMessageW(text_hwnd, EM_REPLACESEL, TRUE, (LPARAM)wline);  // Inserir linha

    // Rolar a janela para exibir a última linha
    SendMessageW(text_hwnd, EM_LINESCROLL, 0, 1);
}

// Função da thread
void* thread_function(void* arg)
{
    ThreadArgs* thread_args = (ThreadArgs*)arg;

    for (int i = 1; i <= NUM_LINES; i++) {
        char line[50];
        sprintf_s(line, sizeof(line), "Thread %d - Linha %d", thread_args->thread_id, i);

        writeLine(thread_args->text_hwnd, line);
    }

    return NULL;
}

int main()
{
    // Inicializar pthreads
    pthread_t threads[NUM_THREADS];
    ThreadArgs thread_args[NUM_THREADS];

    // Obter as dimensões da tela
    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);

    // Calcular as dimensões dos espaços das threads
    int thread_width = screen_width / 2;
    int thread_height = screen_height / 2;

    // Criar e personalizar as janelas das threads
    createThreadWindow(&thread_args[0], 1, 0, 0, thread_width, thread_height);
    createThreadWindow(&thread_args[1], 2, thread_width, 0, thread_width, thread_height);
    createThreadWindow(&thread_args[2], 3, 0, thread_height, thread_width, thread_height);
    createThreadWindow(&thread_args[3], 4, thread_width, thread_height, thread_width, thread_height);

    // Criar as threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_args[i].thread_id = i + 1;
        int result = pthread_create(&threads[i], NULL, thread_function, &thread_args[i]);
        if (result != 0) {
            printf("Falha ao criar a thread %d. Código de erro: %d\n", i + 1, result);
            return 1;
        }
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Aguardar a conclusão de todas as threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Fechar as janelas
    for (int i = 0; i < NUM_THREADS; i++) {
        DestroyWindow(thread_args[i].hwnd);
        UnregisterClass(thread_args[i].hwnd, GetModuleHandle(NULL));
    }

    return 0;
}
