// =====================================================================
// 📊 CASHBOOK PRO - Complete Accounting Application
// Version: 3.0
// Created: 2024
// =====================================================================

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <fstream>
#include <algorithm>
#include <cmath>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")

// ==================== CONSTANTS ====================
#define ID_SAVE 1001
#define ID_PRINT 1002
#define ID_CLEAR 1003
#define ID_EXPORT 1004
#define ID_SEARCH 1005
#define ID_HISTORY 1006
#define ID_THEME 1007
#define ID_SHORTCUT 1008
#define ID_CHARTS 1009

#define WM_TRAYICON (WM_USER + 1)

// ==================== GLOBAL VARIABLES ====================
HWND hOfficeName, hDatePicker;
HWND hReceiptFields[15], hPaymentFields[15];
HWND hTotalReceipt, hTotalPayment, hClosingBalance;
HWND hCurrency500, hCurrency200, hCurrency100, hCurrency50;
HWND hCurrency20, hCurrency10, hCoins, hCItem;
HWND hTotalCash, hStatusBar;
HWND hBtnSave, hBtnPrint, hBtnClear, hBtnExport, hBtnSearch, hBtnHistory, hBtnTheme;
HWND hBtnCharts;
HWND hSearchBox, hHistoryList;
HWND hMainWindow;
HWND hShortcutLabel;

double receiptValues[15] = {0};
double paymentValues[15] = {0};
double openingBalance = 0;

std::vector<std::string> historyData;
std::vector<double> chartData;
bool darkTheme = false;

// ==================== COLOR SCHEMES ====================
COLORREF bgColor = RGB(240, 240, 240);
COLORREF textColor = RGB(0, 0, 0);
COLORREF headerColor = RGB(52, 73, 94);
COLORREF cardColor = RGB(255, 255, 255);
COLORREF borderColor = RGB(200, 200, 200);
COLORREF chartColors[] = {
    RGB(231, 76, 60),   // Red
    RGB(46, 204, 113),  // Green
    RGB(52, 152, 219),  // Blue
    RGB(241, 196, 15),  // Yellow
    RGB(155, 89, 182),  // Purple
    RGB(230, 126, 34),  // Orange
    RGB(26, 188, 156),  // Teal
    RGB(231, 76, 60),   // Pink
    RGB(149, 165, 166), // Gray
    RGB(44, 62, 80)     // Dark Blue
};

COLORREF darkBg = RGB(30, 30, 30);
COLORREF darkText = RGB(220, 220, 220);
COLORREF darkHeader = RGB(20, 30, 50);
COLORREF darkCard = RGB(50, 50, 50);
COLORREF darkBorder = RGB(70, 70, 70);

// ==================== HELPER FUNCTIONS ====================
std::string GetCurrentDate() {
    time_t now = time(nullptr);
    struct tm t;
    localtime_s(&t, &now);
    char buffer[11];
    strftime(buffer, sizeof(buffer), "%d-%m-%Y", &t);
    return std::string(buffer);
}

std::string GetCurrentTime() {
    time_t now = time(nullptr);
    struct tm t;
    localtime_s(&t, &now);
    char buffer[9];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", &t);
    return std::string(buffer);
}

double GetDoubleFromEdit(HWND hEdit) {
    char buffer[32];
    GetWindowTextA(hEdit, buffer, 32);
    if (strlen(buffer) == 0) return 0;
    return atof(buffer);
}

void SetEditText(HWND hEdit, const std::string& text) {
    SetWindowTextA(hEdit, text.c_str());
}

std::string FormatCurrency(double amount) {
    std::stringstream ss;
    ss << "₹" << std::fixed << std::setprecision(2) << amount;
    return ss.str();
}

// ==================== THEME FUNCTIONS ====================
void ApplyTheme(bool dark) {
    darkTheme = dark;
    
    if (dark) {
        bgColor = darkBg;
        textColor = darkText;
        headerColor = darkHeader;
        cardColor = darkCard;
        borderColor = darkBorder;
    } else {
        bgColor = RGB(240, 240, 240);
        textColor = RGB(0, 0, 0);
        headerColor = RGB(52, 73, 94);
        cardColor = RGB(255, 255, 255);
        borderColor = RGB(200, 200, 200);
    }
    
    InvalidateRect(hMainWindow, nullptr, TRUE);
    UpdateWindow(hMainWindow);
    SetWindowTextA(hBtnTheme, dark ? "☀️ Light" : "🌙 Dark");
}

// ==================== CHART DRAWING FUNCTIONS ====================
void DrawPieChart(HDC hdc, RECT rect, std::vector<double> data, std::vector<std::string> labels) {
    if (data.empty()) return;
    
    double total = 0;
    for (double val : data) total += val;
    if (total == 0) return;
    
    int centerX = (rect.left + rect.right) / 2;
    int centerY = (rect.top + rect.bottom) / 2;
    int radius = min((rect.right - rect.left) / 2 - 20, (rect.bottom - rect.top) / 2 - 20);
    
    double startAngle = -90.0;
    
    for (size_t i = 0; i < data.size(); i++) {
        if (data[i] == 0) continue;
        
        double angle = (data[i] / total) * 360.0;
        
        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
        HBRUSH hBrush = CreateSolidBrush(chartColors[i % 10]);
        SelectObject(hdc, hPen);
        SelectObject(hdc, hBrush);
        
        double endAngle = startAngle + angle;
        
        double startRad = (startAngle * 3.14159) / 180.0;
        double endRad = (endAngle * 3.14159) / 180.0;
        
        POINT points[3];
        points[0].x = centerX;
        points[0].y = centerY;
        points[1].x = centerX + (int)(radius * cos(startRad));
        points[1].y = centerY + (int)(radius * sin(startRad));
        points[2].x = centerX + (int)(radius * cos(endRad));
        points[2].y = centerY + (int)(radius * sin(endRad));
        
        POINT polyPoints[20];
        polyPoints[0] = points[0];
        polyPoints[1] = points[1];
        
        int steps = 20;
        for (int j = 0; j <= steps; j++) {
            double t = startAngle + (angle * j / steps);
            double rad = (t * 3.14159) / 180.0;
            polyPoints[j + 2].x = centerX + (int)(radius * cos(rad));
            polyPoints[j + 2].y = centerY + (int)(radius * sin(rad));
        }
        
        Polygon(hdc, polyPoints, steps + 3);
        
        double midAngle = startAngle + (angle / 2);
        double midRad = (midAngle * 3.14159) / 180.0;
        int labelX = centerX + (int)((radius * 0.6) * cos(midRad));
        int labelY = centerY + (int)((radius * 0.6) * sin(midRad));
        
        std::string label = labels[i] + "\n" + FormatCurrency(data[i]);
        RECT textRect = {labelX - 30, labelY - 15, labelX + 30, labelY + 15};
        DrawTextA(hdc, label.c_str(), -1, &textRect, DT_CENTER | DT_NOCLIP);
        
        DeleteObject(hPen);
        DeleteObject(hBrush);
        
        startAngle = endAngle;
    }
}

void DrawBarChart(HDC hdc, RECT rect, std::vector<double> data, std::vector<std::string> labels) {
    if (data.empty()) return;
    
    double maxVal = *std::max_element(data.begin(), data.end());
    if (maxVal == 0) maxVal = 1;
    
    int numBars = data.size();
    int barWidth = min((rect.right - rect.left - 40) / numBars - 5, 80);
    int barSpacing = 5;
    int chartHeight = rect.bottom - rect.top - 80;
    int chartBottom = rect.bottom - 30;
    
    HPEN hPen = CreatePen(PS_SOLID, 2, darkTheme ? RGB(200, 200, 200) : RGB(0, 0, 0));
    SelectObject(hdc, hPen);
    
    MoveToEx(hdc, rect.left + 30, rect.top + 20, nullptr);
    LineTo(hdc, rect.left + 30, chartBottom);
    LineTo(hdc, rect.right - 20, chartBottom);
    
    for (int i = 0; i <= 5; i++) {
        int yPos = chartBottom - (i * chartHeight / 5);
        std::string label = FormatCurrency(maxVal * i / 5);
        RECT labelRect = {rect.left + 2, yPos - 10, rect.left + 28, yPos + 10};
        DrawTextA(hdc, label.c_str(), -1, &labelRect, DT_RIGHT | DT_VCENTER);
        
        HPEN hGridPen = CreatePen(PS_DOT, 1, darkTheme ? RGB(70, 70, 70) : RGB(200, 200, 200));
        SelectObject(hdc, hGridPen);
        MoveToEx(hdc, rect.left + 30, yPos, nullptr);
        LineTo(hdc, rect.right - 20, yPos);
        DeleteObject(hGridPen);
    }
    
    for (size_t i = 0; i < data.size(); i++) {
        int xPos = rect.left + 35 + i * (barWidth + barSpacing);
        int barHeight = (int)((data[i] / maxVal) * chartHeight);
        int yPos = chartBottom - barHeight;
        
        HBRUSH hBrush = CreateSolidBrush(chartColors[i % 10]);
        SelectObject(hdc, hBrush);
        
        RECT barRect = {xPos, yPos, xPos + barWidth, chartBottom};
        FillRect(hdc, &barRect, hBrush);
        
        HPEN hBorderPen = CreatePen(PS_SOLID, 1, darkTheme ? RGB(150, 150, 150) : RGB(0, 0, 0));
        SelectObject(hdc, hBorderPen);
        Rectangle(hdc, xPos, yPos, xPos + barWidth, chartBottom);
        
        DeleteObject(hBrush);
        DeleteObject(hBorderPen);
        
        RECT labelRect = {xPos, chartBottom + 5, xPos + barWidth, chartBottom + 25};
        DrawTextA(hdc, labels[i].c_str(), -1, &labelRect, DT_CENTER | DT_WORDBREAK);
        
        std::string value = FormatCurrency(data[i]);
        RECT valRect = {xPos, yPos - 25, xPos + barWidth, yPos};
        DrawTextA(hdc, value.c_str(), -1, &valRect, DT_CENTER);
    }
    
    DeleteObject(hPen);
}

void DrawLineChart(HDC hdc, RECT rect, std::vector<double> data, std::vector<std::string> labels) {
    if (data.size() < 2) return;
    
    double maxVal = *std::max_element(data.begin(), data.end());
    if (maxVal == 0) maxVal = 1;
    double minVal = *std::min_element(data.begin(), data.end());
    double range = maxVal - minVal;
    if (range == 0) range = 1;
    
    int chartWidth = rect.right - rect.left - 50;
    int chartHeight = rect.bottom - rect.top - 80;
    int chartBottom = rect.bottom - 30;
    int chartLeft = rect.left + 30;
    
    HPEN hPen = CreatePen(PS_SOLID, 2, darkTheme ? RGB(200, 200, 200) : RGB(0, 0, 0));
    SelectObject(hdc, hPen);
    MoveToEx(hdc, chartLeft, rect.top + 20, nullptr);
    LineTo(hdc, chartLeft, chartBottom);
    LineTo(hdc, rect.right - 20, chartBottom);
    
    for (int i = 0; i <= 5; i++) {
        int yPos = chartBottom - (i * chartHeight / 5);
        std::string label = FormatCurrency(maxVal * i / 5);
        RECT labelRect = {rect.left + 2, yPos - 10, chartLeft - 2, yPos + 10};
        DrawTextA(hdc, label.c_str(), -1, &labelRect, DT_RIGHT | DT_VCENTER);
        
        HPEN hGridPen = CreatePen(PS_DOT, 1, darkTheme ? RGB(70, 70, 70) : RGB(200, 200, 200));
        SelectObject(hdc, hGridPen);
        MoveToEx(hdc, chartLeft, yPos, nullptr);
        LineTo(hdc, rect.right - 20, yPos);
        DeleteObject(hGridPen);
    }
    
    HPEN hLinePen = CreatePen(PS_SOLID, 3, RGB(52, 152, 219));
    SelectObject(hdc, hLinePen);
    
    std::vector<POINT> points;
    for (size_t i = 0; i < data.size(); i++) {
        int xPos = chartLeft + (i * chartWidth / (data.size() - 1));
        int yPos = chartBottom - (int)(((data[i] - minVal) / range) * chartHeight);
        points.push_back({xPos, yPos});
        
        HPEN hPointPen = CreatePen(PS_SOLID, 2, RGB(231, 76, 60));
        HBRUSH hPointBrush = CreateSolidBrush(RGB(231, 76, 60));
        SelectObject(hdc, hPointPen);
        SelectObject(hdc, hPointBrush);
        
        Ellipse(hdc, xPos - 5, yPos - 5, xPos + 5, yPos + 5);
        
        DeleteObject(hPointPen);
        DeleteObject(hPointBrush);
        
        if (i < labels.size()) {
            RECT labelRect = {xPos - 20, chartBottom + 5, xPos + 20, chartBottom + 25};
            DrawTextA(hdc, labels[i].c_str(), -1, &labelRect, DT_CENTER);
        }
    }
    
    if (points.size() > 1) {
        Polyline(hdc, points.data(), points.size());
    }
    
    DeleteObject(hLinePen);
    DeleteObject(hPen);
}

// ==================== CHART WINDOW ====================
void ShowCharts() {
    HWND hChartWnd = CreateWindow("STATIC", "📊 Charts & Analytics - CashBook Pro",
                                  WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_VSCROLL,
                                  CW_USEDEFAULT, CW_USEDEFAULT, 1000, 700,
                                  nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
    
    if (!hChartWnd) return;
    
    if (darkTheme) {
        SetClassLongPtr(hChartWnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(darkBg));
    }
    
    HWND hTab = CreateWindow(WC_TABCONTROL, "", WS_CHILD | WS_VISIBLE | TCS_TABS,
                             10, 10, 960, 30, hChartWnd, nullptr, GetModuleHandle(nullptr), nullptr);
    
    TCITEM tie = {0};
    tie.mask = TCIF_TEXT;
    
    tie.pszText = "Pie Chart";
    TabCtrl_InsertItem(hTab, 0, &tie);
    tie.pszText = "Bar Chart";
    TabCtrl_InsertItem(hTab, 1, &tie);
    tie.pszText = "Line Chart";
    TabCtrl_InsertItem(hTab, 2, &tie);
    tie.pszText = "Summary";
    TabCtrl_InsertItem(hTab, 3, &tie);
    
    HWND hChartDisplay = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
                                     10, 50, 960, 550, hChartWnd, nullptr, GetModuleHandle(nullptr), nullptr);
    
    HWND hClose = CreateWindow("BUTTON", "✖ Close", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                              430, 610, 120, 35, hChartWnd, (HMENU)1, GetModuleHandle(nullptr), nullptr);
    
    HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    SendMessage(hTab, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hClose, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    std::vector<double> chartData;
    std::vector<std::string> chartLabels;
    
    const char* receiptHeads[] = {
        "Opening", "Sales", "Service", "Interest",
        "Commission", "Rent", "Dividend", "Refund",
        "Loan", "Misc", "Other", "Gift",
        "Investment", "Royalty", "Consulting"
    };
    
    for (int i = 0; i < 15; i++) {
        if (receiptValues[i] > 0) {
            chartData.push_back(receiptValues[i]);
            chartLabels.push_back(receiptHeads[i]);
        }
    }
    
    const char* paymentHeads[] = {
        "Purchases", "Salary", "Rent", "Utilities",
        "Transport", "Entertainment", "Supplies",
        "Repairs", "Insurance", "Loan", "Misc",
        "Travel", "Marketing", "Taxes", "Depreciation"
    };
    
    for (int i = 0; i < 15; i++) {
        if (paymentValues[i] > 0) {
            chartData.push_back(paymentValues[i]);
            chartLabels.push_back(paymentHeads[i]);
        }
    }
    
    ::chartData = chartData;
    
    WNDPROC oldProc = (WNDPROC)SetWindowLongPtr(hChartDisplay, GWLP_WNDPROC, 
        [](HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) -> LRESULT {
            if (msg == WM_PAINT) {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                
                RECT rect;
                GetClientRect(hwnd, &rect);
                
                HBRUSH hBgBrush = CreateSolidBrush(darkTheme ? darkBg : RGB(255, 255, 255));
                FillRect(hdc, &rect, hBgBrush);
                DeleteObject(hBgBrush);
                
                HWND hTabParent = GetParent(hwnd);
                HWND hTab = GetDlgItem(hTabParent, 1000);
                int tabIndex = TabCtrl_GetCurSel(hTab);
                
                if (!::chartData.empty()) {
                    const char* labels[] = {
                        "Opening", "Sales", "Service", "Interest",
                        "Commission", "Rent", "Dividend", "Refund",
                        "Loan", "Misc", "Other", "Gift",
                        "Investment", "Royalty", "Consulting",
                        "Purchases", "Salary", "Rent", "Utilities",
                        "Transport", "Entertainment", "Supplies",
                        "Repairs", "Insurance", "Loan", "Misc",
                        "Travel", "Marketing", "Taxes", "Depreciation"
                    };
                    
                    std::vector<std::string> chartLabels;
                    for (size_t i = 0; i < ::chartData.size() && i < 30; i++) {
                        chartLabels.push_back(labels[i]);
                    }
                    
                    rect.left += 10;
                    rect.right -= 10;
                    rect.top += 10;
                    rect.bottom -= 10;
                    
                    switch (tabIndex) {
                        case 0:
                            DrawPieChart(hdc, rect, ::chartData, chartLabels);
                            break;
                        case 1:
                            DrawBarChart(hdc, rect, ::chartData, chartLabels);
                            break;
                        case 2:
                            DrawLineChart(hdc, rect, ::chartData, chartLabels);
                            break;
                        case 3: {
                            char buffer[2048];
                            double totalReceipt = GetDoubleFromEdit(hTotalReceipt);
                            double totalPayment = GetDoubleFromEdit(hTotalPayment);
                            double closingBalance = GetDoubleFromEdit(hClosingBalance);
                            double totalCash = GetDoubleFromEdit(hTotalCash);
                            
                            sprintf_s(buffer, 
                                "📊 SUMMARY REPORT\n"
                                "═══════════════════════════════════════\n\n"
                                "📥 Total Receipts:    %s\n"
                                "📤 Total Payments:    %s\n"
                                "💰 Closing Balance:   %s\n"
                                "💵 Total Cash:        %s\n\n"
                                "📈 Transaction Count: %d\n"
                                "📊 Categories: %d\n"
                                "⭐ Average Receipt:   %s\n"
                                "⭐ Average Payment:   %s\n\n"
                                "📌 Status: %s\n"
                                "═══════════════════════════════════════",
                                FormatCurrency(totalReceipt).c_str(),
                                FormatCurrency(totalPayment).c_str(),
                                FormatCurrency(closingBalance).c_str(),
                                FormatCurrency(totalCash).c_str(),
                                (int)::chartData.size(),
                                (int)::chartData.size() / 2,
                                FormatCurrency(totalReceipt / max(1, (int)::chartData.size())).c_str(),
                                FormatCurrency(totalPayment / max(1, (int)::chartData.size())).c_str(),
                                closingBalance >= 0 ? "🟢 PROFIT" : "🔴 LOSS"
                            );
                            
                            RECT textRect = {rect.left + 50, rect.top + 50, rect.right - 50, rect.bottom - 50};
                            HFONT hFont = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                                    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                                    DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Courier New");
                            SelectObject(hdc, hFont);
                            SetTextColor(hdc, darkTheme ? RGB(220, 220, 220) : RGB(0, 0, 0));
                            SetBkMode(hdc, TRANSPARENT);
                            DrawTextA(hdc, buffer, -1, &textRect, DT_LEFT);
                            DeleteObject(hFont);
                            break;
                        }
                    }
                } else {
                    std::string msg = "📭 No data available for charts.\nPlease enter some transactions first.";
                    RECT textRect;
                    GetClientRect(hwnd, &textRect);
                    SetTextColor(hdc, darkTheme ? RGB(200, 200, 200) : RGB(100, 100, 100));
                    SetBkMode(hdc, TRANSPARENT);
                    DrawTextA(hdc, msg.c_str(), -1, &textRect, DT_CENTER | DT_VCENTER);
                }
                
                EndPaint(hwnd, &ps);
                return 0;
            }
            return DefWindowProc(hwnd, msg, wp, lp);
        }
    );
    
    SetWindowLongPtr(hTab, GWLP_
