#include "stdafx.h"
#include <strsafe.h>
#include "programmer-ui.h"
#include "DownloaderDlg.h"
#include "ProgramDlg.h"
#include "UploadDlg.h"
#include "MemoryDumpDlg.h"

#include <initguid.h>
#include <devguid.h>
#include <setupapi.h>

#pragma comment(lib, "Setupapi.lib")

#define TIMER_IDENTIFY			(0x5424)
#define TIMER_CHECK_DEVICE		(0x5425)


////////////////////////////////////////////////////////////////////////////////////
//

static void ShowLastError(LPTSTR lpszFunction, DWORD dwLastError)
{
	// Retrieve the system error message for the last-error code
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwLastError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dwLastError, lpMsgBuf);

	AfxMessageBox((LPCTSTR)lpDisplayBuf, MB_ICONERROR | MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}

static int EnumSerialPorts(std::list<SerialPort> & ports)
{
	//
	HDEVINFO hDeviceInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS,
		NULL,
		NULL,
		DIGCF_PRESENT);

	if (hDeviceInfo == INVALID_HANDLE_VALUE)
		return -1;

	//
	for (int nDevice = 0; ; nDevice++)
	{
		SP_DEVINFO_DATA devInfoData;

		ZeroMemory(&devInfoData, sizeof(devInfoData));
		devInfoData.cbSize = sizeof(devInfoData);

		BOOL bRet = SetupDiEnumDeviceInfo(hDeviceInfo, nDevice, &devInfoData);
		if (!bRet)
			break;

		DWORD dwSize = 0;
		CString strName;

		SetupDiGetDeviceRegistryProperty(hDeviceInfo, &devInfoData, SPDRP_FRIENDLYNAME, NULL, NULL, 0, &dwSize);
		SetupDiGetDeviceRegistryProperty(hDeviceInfo, &devInfoData, SPDRP_FRIENDLYNAME, NULL, (PBYTE)strName.GetBuffer(dwSize), dwSize, NULL);
		strName.ReleaseBuffer();

		HKEY hDeviceKey = SetupDiOpenDevRegKey(hDeviceInfo, &devInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_QUERY_VALUE);
		if (hDeviceKey)
		{
			TCHAR pszPortName[32];
			DWORD dwSize = sizeof(pszPortName);
			DWORD dwType = 0;

			if ((RegQueryValueEx(hDeviceKey, _T("PortName"), NULL, &dwType, (LPBYTE)pszPortName, &dwSize) == ERROR_SUCCESS) && (dwType == REG_SZ))
			{
				if (_tcsnicmp(pszPortName, _T("COM"), 3) == 0)
				{
					int nPort = _ttoi(pszPortName + 3);
					if (nPort != 0)
					{
						ports.push_back(SerialPort(nPort, strName));
					}
				}
			}

			RegCloseKey(hDeviceKey);
		}
	}

	SetupDiDestroyDeviceInfoList(hDeviceInfo);

	return ports.size();
}

static CSerial::EBaudrate	Map_BaudRate[] =
{
	CSerial::EBaud9600,
	CSerial::EBaud14400,
	CSerial::EBaud19200,
	CSerial::EBaud38400,
	CSerial::EBaud56000,
	CSerial::EBaud57600,
	CSerial::EBaud115200,
};

static CSerial::EDataBits	Map_DataBits[] =
{
	CSerial::EData5,
	CSerial::EData6,
	CSerial::EData7,
	CSerial::EData8,
};

static CSerial::EParity		Map_Parity[] =
{
	CSerial::EParNone,
	CSerial::EParOdd,
	CSerial::EParEven,
	CSerial::EParMark,
	CSerial::EParSpace,
};

static CSerial::EStopBits	Map_StopBits[] =
{
	CSerial::EStop1,
	CSerial::EStop1_5,
	CSerial::EStop2,
};

static CSerial::EHandshake	Map_FlowControl[] =
{
	CSerial::EHandshakeOff,
	CSerial::EHandshakeSoftware,
	CSerial::EHandshakeHardware,
};



////////////////////////////////////////////////////////////////////////////////////
// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	//
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

//
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////////////////
//

CDownloaderDlg::CDownloaderDlg(CWnd* pParent)
	: CDialogEx(IDD_DOWNLOADER, pParent)
	, m_bConnected(FALSE)
	, m_State(_READY)
	, m_nTimerID(0)
	, m_strPortName(_T(""))
	, m_strDevID("")
	, m_strBootVer(_T(""))
	, m_strFWVer("")
	, m_nDevID(0)
	, m_nBootVer(0)
	, m_nFWVer(0)
	, m_pPacketListener(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_sPortNum = AfxGetApp()->GetProfileIntA(_T("SerialSettings"), _T("PortNum"), 1);
	m_sBaudRate = (CSerial::EBaudrate)AfxGetApp()->GetProfileIntA(_T("SerialSettings"), _T("BaudRate"), CSerial::EBaud57600);
	m_sDataBits = (CSerial::EDataBits)AfxGetApp()->GetProfileIntA(_T("SerialSettings"), _T("DataBits"), CSerial::EData8);
	m_sParity = (CSerial::EParity)AfxGetApp()->GetProfileIntA(_T("SerialSettings"), _T("Parity"), CSerial::EParNone);
	m_sStopBits = (CSerial::EStopBits)AfxGetApp()->GetProfileIntA(_T("SerialSettings"), _T("StopBits"), CSerial::EStop1);
	m_sHandshake = (CSerial::EHandshake)AfxGetApp()->GetProfileIntA(_T("SerialSettings"), _T("Handshake"), CSerial::EHandshakeOff);

	m_bVerbo = AfxGetApp()->GetProfileIntA(_T("LogSettings"), _T("Verbo"), 0);

}

CDownloaderDlg::~CDownloaderDlg()
{
}


void CDownloaderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_DEV_ID, m_strDevID);
	DDX_Text(pDX, IDC_BOOT_VER, m_strBootVer);
	DDX_Text(pDX, IDC_FW_VER, m_strFWVer);
	DDX_Control(pDX, IDC_LOG_HISTORY, m_wndHistory);

	DDX_Control(pDX, IDC_SERIAL_PORT, m_wndSerialPort);
	DDX_Control(pDX, IDC_SERIAL_BAUDRATE, m_wndSerialBaudRate);
	DDX_Control(pDX, IDC_SERIAL_DATABITS, m_wndSerialDataBits);
	DDX_Control(pDX, IDC_SERIAL_PARITY, m_wndSerialParity);
	DDX_Control(pDX, IDC_SERIAL_STOPBITS, m_wndSerialStopBits);
	DDX_Control(pDX, IDC_SERIAL_HANDSHAKE, m_wndSerialHandshake);

	DDX_Check(pDX, IDC_LOG_DETAIL, m_bVerbo);
}


BEGIN_MESSAGE_MAP(CDownloaderDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_REGISTERED_MESSAGE(CSerialWnd::mg_nDefaultComMsg, OnSerialMessage)
	ON_BN_CLICKED(IDC_CONNECT, OnConnect)
	ON_BN_CLICKED(IDC_DOWNLOAD, OnDownload)
	ON_BN_CLICKED(IDC_UPLOAD, OnUpload)
	ON_BN_CLICKED(IDC_MEMORY, OnMemoryDump)
	ON_BN_CLICKED(IDC_LOG_CLEAR, OnClearLog)
	ON_BN_CLICKED(IDC_RUN_USERPROG, OnRunUserProgram)
	ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
	ON_BN_CLICKED(IDC_LOG_DETAIL, OnChangeLogLevel)
	ON_BN_CLICKED(IDC_DEV_CHECK, OnCheckDevice)
END_MESSAGE_MAP()


BOOL CDownloaderDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	//
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	//
	m_wndHistory.SetExtendedStyle(m_wndHistory.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
	m_wndHistory.InsertColumn(0, _T("Log history"), LVCFMT_LEFT, 380, 0);

	//
	RefillSerialPort();

	//
	UpdateData(FALSE);

	return TRUE;
}

void CDownloaderDlg::OnOK()
{
}

void CDownloaderDlg::OnCancel()
{
	if (m_State == _READY || m_State == _RUN)
	{
		if (m_State == _RUN)
		{
			SendStartCommand();
			Sleep(200);
			Disconnect();
		}

		CDialogEx::OnCancel();
	}
	else
	{
		::MessageBeep((UINT)-1);
	}
}

void CDownloaderDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CDownloaderDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CDownloaderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDownloaderDlg::OnTimer(UINT nIDEvent)
{
	CDialog::OnTimer(nIDEvent);

	if (m_State == _CHECK_DEVICE)
	{
		KillTimer(m_nTimerID);
		m_nTimerID = 0;

		// no reponse!!
		::MessageBeep(-1);
		Log(_INFO, "Device not reponse!");

		Disconnect();
	}
	else if (m_State == _IDENTIFY_DEVICE)
	{
		KillTimer(m_nTimerID);
		m_nTimerID = 0;

		// no reponse!!
		::MessageBeep(-1);
		Log(_INFO, "Device not reponse!");

		// continue
		SendRebootRequest();

		m_State = _IDENTIFY_BOOTLOADER;
		m_SubState = 0;
		m_nTimerID = SetTimer(TIMER_IDENTIFY, 500, NULL);
	}
	else if (m_State == _IDENTIFY_BOOTLOADER)
	{
		if (m_SubState < 5)
		{
			m_SubState += 1;

			SendIdentify();
		}
		else
		{
			KillTimer(m_nTimerID);
			m_nTimerID = 0;

			// no reponse!!
			::MessageBeep(-1);
			Log(_INFO, "Bootloader not reponse!!");

			Disconnect();
		}
	}
}

LRESULT CDownloaderDlg::OnSerialMessage(WPARAM wParam, LPARAM lParam)
{
	CSerial::EEvent eEvent = CSerial::EEvent(LOWORD(wParam));
	CSerial::EError eError = CSerial::EError(HIWORD(wParam));

	if (eEvent & CSerial::EEventRecv)
	{
		CHAR ch;
		DWORD dwRead;

		while (m_Serial.Read(&ch, sizeof(ch), &dwRead) == ERROR_SUCCESS && dwRead == sizeof(ch))
		{
			if (m_State == _CHECK_DEVICE || m_State == _IDENTIFY_DEVICE)
			{
				// parser incomming device response
				if (m_LineBuf.push(ch))
				{
					VResponse r = VResponse::parse(m_LineBuf.getLine(), m_LineBuf.getLineLength());

					if (r.getCode() != VCMD_INVALID)
					{
						//CString str;
						//r.toString(str.GetBuffer(512), 512);
						//str.ReleaseBuffer();
						//
						//Log(_VERBO, "  recv %4d bytes %s", str.GetLength(), GetDigest((void *)(LPCTSTR)str, str.GetLength()));
						Log(_VERBO, "  recv [%4d bytes] %s", m_LineBuf.getLineLength(), GetDigest(m_LineBuf.getLine(), m_LineBuf.getLineLength()));
						OnVResponseReceived(&r);
					}
				}
			}
			else if (m_State == _IDENTIFY_BOOTLOADER || m_State == _RUN)
			{
				if (m_Parser.push(ch))
				{
					BPacket * pPacket = new BPacket;
					m_Parser.getPacket(pPacket);

					Log(_VERBO, "  recv [%4d bytes] %s", pPacket->payloadLen + 5, GetDigest(pPacket));
					Route(pPacket);

					delete pPacket;
				}
			}
		}
	}
	else if (eEvent & CSerial::EEventSend)
	{
		TRACE("Last character on output was sent");
	}

	return 0L;
}

void CDownloaderDlg::OnConnect()
{
	if (m_State == _READY)
	{
		UpdateData(TRUE);

		Connect(FALSE);
	}
	else
	{
		Disconnect();
	}
}

void CDownloaderDlg::Connect(BOOL bDeviceCheck)
{
	//
	if (m_sPortNum > 0)
	{
		LRESULT lResult;

		AfxGetApp()->WriteProfileInt(_T("SerialSettings"), _T("PortNum"), m_sPortNum);
		AfxGetApp()->WriteProfileInt(_T("SerialSettings"), _T("BaudRate"), m_sBaudRate);
		AfxGetApp()->WriteProfileInt(_T("SerialSettings"), _T("DataBits"), m_sDataBits);
		AfxGetApp()->WriteProfileInt(_T("SerialSettings"), _T("Parity"), m_sParity);
		AfxGetApp()->WriteProfileInt(_T("SerialSettings"), _T("StopBits"), m_sStopBits);
		AfxGetApp()->WriteProfileInt(_T("SerialSettings"), _T("Handshake"), m_sHandshake);

		if ((lResult = OpenSerial(m_strPortName, m_sBaudRate, m_sDataBits, m_sParity, m_sStopBits, m_sHandshake)) == ERROR_SUCCESS)
		{
			Log(_INFO, "Open serial port(COM%d)", m_sPortNum);

			if (bDeviceCheck)
			{
				//
				m_State = _CHECK_DEVICE;
				m_SubState = 0;

				m_LineBuf.reset();

				// query device firmware version
				SendFirmwareVersion();

				m_nTimerID = SetTimer(TIMER_CHECK_DEVICE, 2000, NULL);
			}
			else
			{
				//
				m_State = _IDENTIFY_DEVICE;
				m_SubState = 0;

				m_Parser.reset();

				// query device firmware version
				SendFirmwareVersion();

				m_nTimerID = SetTimer(TIMER_IDENTIFY, 1000, NULL);
			}

			//
			UpdateControls();
		}
		else
		{
			Log(_INFO, "Serial port open failed: %X", lResult);

			ShowLastError(_T("Serial port open"), lResult);
		}
	}
}

void CDownloaderDlg::Disconnect()
{
	//
	CloseSerial();

	//
	m_State = _READY;

	//
	m_nDevID = 0;
	m_nBootVer = 0;
	m_nFWVer = 0;

	UpdateData(FALSE);
	UpdateControls();
}


void CDownloaderDlg::UpdateTitle()
{
	CString strTitle;

	//if (m_bConnected)
	if (m_State != _READY)
	{
		int b = 0, d = 0;
		char p = ' ';
		char * s = "";

		switch (m_sBaudRate)
		{
		case CSerial::EBaud9600: b = 9600;		break;
		case CSerial::EBaud14400: b = 14400;	break;
		case CSerial::EBaud19200: b = 19200;	break;
		case CSerial::EBaud38400: b = 38400;	break;
		case CSerial::EBaud56000: b = 56000;	break;
		case CSerial::EBaud57600: b = 57600;	break;
		case CSerial::EBaud115200: b = 115200;	break;
		}

		switch (m_sDataBits)
		{
		case CSerial::EData5: d = 5; break;
		case CSerial::EData6: d = 6; break;
		case CSerial::EData7: d = 7; break;
		case CSerial::EData8: d = 8; break;
		}

		switch (m_sParity)
		{
		case CSerial::EParNone: p = 'N'; break;
		case CSerial::EParOdd: p = 'O'; break;
		case CSerial::EParEven: p = 'E'; break;
		case CSerial::EParMark: p = 'M'; break;
		case CSerial::EParSpace: p = 'S'; break;
		}

		switch (m_sStopBits)
		{
		case CSerial::EStop1: s = "1";	break;
		case CSerial::EStop1_5: s = "1.5"; break;
		case CSerial::EStop2: s = "2";	break;
		}

		strTitle.Format(_T("STM32F1 Programmer - COM%d:%d,%d,%c,%s"), m_sPortNum, b, d, p, s);
	}
	else
	{
		strTitle.Format("STM32F1 Programmer - Unconnected!");
	}

	//
	GetDlgItem(IDC_CONNECT)->SetWindowTextA((m_State != _READY ? _T("Disconnect") : _T("Connect")));
	//
	SetWindowText(strTitle);
}

void CDownloaderDlg::UpdateControls()
{
	m_wndSerialPort.EnableWindow(m_State != _READY ? FALSE : TRUE);
	m_wndSerialBaudRate.EnableWindow(m_State != _READY ? FALSE : TRUE);
	m_wndSerialDataBits.EnableWindow(m_State != _READY ? FALSE : TRUE);
	m_wndSerialParity.EnableWindow(m_State != _READY ? FALSE : TRUE);
	m_wndSerialStopBits.EnableWindow(m_State != _READY ? FALSE : TRUE);
	m_wndSerialHandshake.EnableWindow(m_State != _READY ? FALSE : TRUE);
}

BOOL CDownloaderDlg::UpdateData(BOOL bSaveAndValidate)
{
	if (!bSaveAndValidate)
	{
		// Port
		if (m_sPortNum)
		{
			int nPortIdx = 0;

			for (std::list<SerialPort>::iterator it = m_SerialPorts.begin(); it != m_SerialPorts.end(); it++)
			{
				if (m_sPortNum == (*it).m_nPort)
				{
					m_wndSerialPort.SetCurSel(nPortIdx);
					break;
				}

				nPortIdx++;
			}
		}

		// BaudRate
		for (int i = 0; i < sizeof(Map_BaudRate) / sizeof(Map_BaudRate[0]); i++)
		{
			if (Map_BaudRate[i] == m_sBaudRate)
			{
				m_wndSerialBaudRate.SetCurSel(i);
				break;
			}
		}
		// DataBits
		for (int i = 0; i < sizeof(Map_DataBits) / sizeof(Map_DataBits[0]); i++)
		{
			if (Map_DataBits[i] == m_sDataBits)
			{
				m_wndSerialDataBits.SetCurSel(i);
				break;
			}
		}
		// Parity
		for (int i = 0; i < sizeof(Map_Parity) / sizeof(Map_Parity[0]); i++)
		{
			if (Map_Parity[i] == m_sParity)
			{
				m_wndSerialParity.SetCurSel(i);
				break;
			}
		}
		// StopBits
		for (int i = 0; i < sizeof(Map_StopBits) / sizeof(Map_StopBits[0]); i++)
		{
			if (Map_StopBits[i] == m_sStopBits)
			{
				m_wndSerialStopBits.SetCurSel(i);
				break;
			}
		}
		// Handshake(FlowControl)
		for (int i = 0; i < sizeof(Map_FlowControl) / sizeof(Map_FlowControl[0]); i++)
		{
			if (Map_FlowControl[i] == m_sHandshake)
			{
				m_wndSerialHandshake.SetCurSel(i);
				break;
			}
		}

		//
		m_strDevID.Format("%04X", m_nDevID);
		m_strBootVer.Format("%04X", m_nBootVer);
		m_strFWVer.Format("%04X", m_nFWVer);

		//
		UpdateTitle();
	}

	BOOL bRet = CDialog::UpdateData(bSaveAndValidate);

	if (bSaveAndValidate)
	{
		int index;

		// Port
		if ((index = m_wndSerialPort.GetCurSel()) >= 0)
		{
			std::list<SerialPort>::iterator it = m_SerialPorts.begin();

			while (index)
			{
				it++;
				index--;
			}

			m_strPortName.Format(_T("\\\\.\\COM%d"), (*it).m_nPort);
			m_sPortNum = (*it).m_nPort;
		}

		// BaudRate
		if ((index = m_wndSerialBaudRate.GetCurSel()) >= 0)
			m_sBaudRate = Map_BaudRate[index];
		// DataBits
		if ((index = m_wndSerialDataBits.GetCurSel()) >= 0)
			m_sDataBits = Map_DataBits[index];
		// Parity
		if ((index = m_wndSerialParity.GetCurSel()) >= 0)
			m_sParity = Map_Parity[index];
		// StopBits
		if ((index = m_wndSerialStopBits.GetCurSel()) >= 0)
			m_sStopBits = Map_StopBits[index];
		// Handshake(FlowControl)
		if ((index = m_wndSerialHandshake.GetCurSel()) >= 0)
			m_sHandshake = Map_FlowControl[index];
	}

	return bRet;
}

LRESULT CDownloaderDlg::OpenSerial(LPCTSTR lpszDevice, CSerial::EBaudrate nBaudRate, CSerial::EDataBits nDataBits, CSerial::EParity nParity, CSerial::EStopBits nStopBits, CSerial::EHandshake nHandshake)
{
	CWaitCursor wait;
	LONG lResult;

	if ((lResult = m_Serial.Open(lpszDevice, m_hWnd)) == ERROR_SUCCESS)
	{
		//
		m_Serial.Setup(nBaudRate, nDataBits, nParity, nStopBits);
		m_Serial.SetupHandshaking(nHandshake);
		m_Serial.SetupReadTimeouts(CSerial::EReadTimeoutNonblocking);
	}

	return lResult;
}

/*
int CDownloaderDlg::OpenSerial(LPCTSTR lpszDevice, CSerial::EBaudrate nBaudRate, CSerial::EDataBits nDataBits, CSerial::EParity nParity, CSerial::EStopBits nStopBits, CSerial::EHandshake nHandshake)
{
	CWaitCursor wait;
	LONG lResult;

	if ((lResult = m_Serial.Open(lpszDevice, m_hWnd)) == ERROR_SUCCESS)
	{
		//
		m_Serial.Setup(nBaudRate, nDataBits, nParity, nStopBits);
		m_Serial.SetupHandshaking(nHandshake);
		m_Serial.SetupReadTimeouts(CSerial::EReadTimeoutNonblocking);

		Log("serial connected...");

		//
		m_State = _IDENTIFY;
		m_subState = 0;

		// set timer to identify device
		//   on timer -> first send reset request to user application
		//               and send identify command to bootloader after 500ms 
		Log("start device identify");
		m_nTimerID = SetTimer(TIMER_IDENTIFY, 500, NULL);
	}
	else
	{
		Log("connection failed: %X", lResult);
		ShowLastError(_T("Serial connection"), lResult);
	}
}
*/

void CDownloaderDlg::CloseSerial()
{
	m_Serial.Close();

	if (m_nTimerID)
	{
		KillTimer(m_nTimerID);
		m_nTimerID = 0;
	}

	Log(_INFO, "Close serial port!");
}

void CDownloaderDlg::OnDownload()
{
	if (m_State == _RUN)
	{
		CProgramDlg dlg(this);

		m_pPacketListener = (BPacketListener *)&dlg;
		INT_PTR ret = dlg.DoModal();
		m_pPacketListener = (BPacketListener *)this;

		if (ret == IDYES)
			Disconnect();
	}
	else
	{
		::MessageBeep((UINT)-1);
	}
}

void CDownloaderDlg::OnUpload()
{
	if (m_State == _RUN)
	{
		CUploadDlg dlg(this);

		m_pPacketListener = (BPacketListener *)&dlg;
		INT_PTR ret = dlg.DoModal();
		m_pPacketListener = (BPacketListener *)this;
	}
	else
	{
		::MessageBeep((UINT)-1);
	}
}

void CDownloaderDlg::OnMemoryDump()
{
	if (m_State == _RUN)
	{
		CMemoryDumpDlg dlg(this);

		m_pPacketListener = (BPacketListener *)&dlg;
		INT_PTR ret = dlg.DoModal();
		m_pPacketListener = (BPacketListener *)this;
	}
	else
	{
		::MessageBeep((UINT)-1);
	}
}

void CDownloaderDlg::SendFirmwareVersion()
{
	//if (m_State != _IDENTIFY_DEVICE && m_State != _CHECK_DEVICE)
	//	return;

	//
	Log(_INFO, "Query device firmware version.");
	this->SendCommand((uint8_t *)"#FV\r\n", 5);
}

void CDownloaderDlg::SendRebootRequest()
{
	//if (m_State != _IDENTIFY_DEVICE)
	//	return;

	// "#RB\r\n"
	Log(_INFO, "Request device reboot to enter bootloader mode");
	this->SendCommand((uint8_t *)"#RB\r\n", 5);
}

void CDownloaderDlg::SendIdentify()
{
	//if (m_State != _IDENTIFY_BOOTLOADER)
	//	return;

	//
	BPacketMaker maker;

	maker.start(HCODE_IDENTIFY);
	maker.finish();

	//
	Log(_INFO, "Send bootloader identify command: #%d", m_SubState);
	this->SendCommand(maker.get_data(), maker.get_size());
}

void CDownloaderDlg::SendStartCommand()
{
	BPacketMaker maker;

	maker.start(HCODE_START);
	maker.finish();

	this->SendCommand(maker.get_data(), maker.get_size());
}

void CDownloaderDlg::Log(LogLevel level, LPCTSTR format, ...)
{
	if (m_bVerbo || level != _VERBO)
	{
		TCHAR buf[1024];
		va_list	va;

		va_start(va, format);
		vsprintf_s(buf, format, va);
		va_end(va);

		m_wndHistory.InsertItem(m_wndHistory.GetItemCount(), buf);
		m_wndHistory.EnsureVisible(m_wndHistory.GetItemCount() - 1, FALSE);
	}
}

void CDownloaderDlg::Route(BPacket * pPacket)
{
	if (m_pPacketListener)
		m_pPacketListener->OnBPacketReceived(pPacket);
	else
		this->OnBPacketReceived(pPacket);
}

void CDownloaderDlg::OnBPacketReceived(BPacket * pPacket)
{
	TRACE("> CODE : %02X\n", pPacket->code);
	switch (pPacket->code)
	{
	case DCODE_IDENTIFY:
		TRACE("    Dev-ID: %04X\n", pPacket->i.devId);
		TRACE("    F/W ver: %04X\n", pPacket->i.fwVer);

		if (m_State == _IDENTIFY_BOOTLOADER)
		{
			m_State = _RUN;

			m_nDevID = pPacket->i.devId;
			m_nBootVer = pPacket->i.fwVer;

			KillTimer(m_nTimerID);
			UpdateData(FALSE);

			Log(_INFO, "Device identified!");
		}
		break;
	case DCODE_ACK:
		break;
	case DCODE_NACK:
		TRACE("    REASON: %04X", pPacket->e.error);
		break;
	case DCODE_DUMP_MEM:
		//uint32_t addr = ((uint32_t)res.data[3] << 24) + ((uint32_t)res.data[2] << 16) + ((uint32_t)res.data[1] << 8) + ((uint32_t)res.data[0]);
		//TRACE("> ADDR : %08X\n", addr);
		TRACE("    ADDR: %08X\n", pPacket->d.addr);

		{
			char sz[3 * 16 + 256];

			for (int n = 4; n < pPacket->payloadLen; n += 16)
			{
				uint8_t * data = &pPacket->data[n];
				int i = 0;

				sprintf_s(sz, sizeof(sz), "%08X   %02X %02X %02X %02X %02X %02X %02X %02X - %02X %02X %02X %02X %02X %02X %02X %02X\n",
					n - 4,
					data[0x00], data[0x01], data[0x02], data[0x03], data[0x04], data[0x05], data[0x06], data[0x07],
					data[0x08], data[0x09], data[0x0A], data[0x0B], data[0x0C], data[0x0D], data[0x0E], data[0x0F]);
				TRACE("    %s", sz);
			}
		}
		break;
	}
}

void CDownloaderDlg::OnVResponseReceived(VResponse * pResponse)
{
	//
	if (m_State == _CHECK_DEVICE || m_State == _IDENTIFY_DEVICE)
	{
		if (pResponse->getCode() == VCMD_FIRMWARE_VERSION)
		{
			//
			KillTimer(m_nTimerID);
			m_nTimerID = 0;

			//
			m_nFWVer = pResponse->getNumber(1);
			UpdateData(FALSE);

			//
			Log(_INFO, "Device firmware version: 0x%04X", m_nFWVer);


			if (m_State == _IDENTIFY_DEVICE)
			{
				//
				SendRebootRequest();

				//
				m_State = _IDENTIFY_BOOTLOADER;
				m_SubState = 0;
				m_nTimerID = SetTimer(TIMER_IDENTIFY, 500, NULL);
			}
			else
			{
				//
				Disconnect();
			}
		}
	}
}

void CDownloaderDlg::SendCommand(uint8_t * pData, uint16_t nDataLen)
{
	Log(_VERBO, "  send [%4d bytes] %s", nDataLen, GetDigest(pData, nDataLen));

	m_Serial.Write(pData, nDataLen);
}

CString CDownloaderDlg::GetDigest(BPacket * pPacket)
{
	// SOF, pPacket->code, pPacket->payloadLen(LSB), pPacket->payloadLen(MSB), pPacket->payload ...

	CString str;
	LPTSTR ptr = str.GetBuffer(1024);

	ptr += wsprintf(ptr, _T("{ "));
	ptr += wsprintf(ptr, _T("%02X "), PACKET_SOF);
	ptr += wsprintf(ptr, _T("%02X "), pPacket->code);
	ptr += wsprintf(ptr, _T("%02X "), pPacket->payloadLen >> 8);
	ptr += wsprintf(ptr, _T("%02X "), pPacket->payloadLen & 0xFF);

	uint8_t * data = (uint8_t *)&pPacket->data[0];
	int len = (pPacket->payloadLen < 6 ? pPacket->payloadLen : 6);
	for (int i = 0; i < len; i++)
		ptr += wsprintf(ptr, "%02X ", data[i]);
	if (len < pPacket->payloadLen)
		ptr += wsprintf(ptr, _T("... "));
	wsprintf(ptr, _T("}"));
	str.ReleaseBuffer();

	return str;

}

CString CDownloaderDlg::GetDigest(void * pData, uint16_t nDataLen)
{
	CString str;
	uint8_t * data = (uint8_t *)pData;

	LPTSTR ptr = str.GetBuffer(1024);
	int len = (nDataLen < 10 ? nDataLen : 10);

	ptr += wsprintf(ptr, _T("{ "));
	for (int i = 0; i < len; i++)
		ptr += wsprintf(ptr, "%02X ", data[i]);
	if (len < nDataLen)
		ptr += wsprintf(ptr, _T("... "));
	wsprintf(ptr, _T("}"));
	str.ReleaseBuffer();

	return str;
}

void CDownloaderDlg::OnClearLog()
{
	m_wndHistory.DeleteAllItems();
}

void CDownloaderDlg::OnRunUserProgram()
{
	if (m_State == _RUN)
	{
		SendStartCommand();
		Disconnect();
	}
	else
	{
		MessageBeep((UINT)-1);
	}
}

void CDownloaderDlg::OnRefresh()
{
	UpdateData(TRUE);

	RefillSerialPort();
}

void CDownloaderDlg::RefillSerialPort()
{
	//
	m_wndSerialPort.ResetContent();

	m_SerialPorts.clear();
	EnumSerialPorts(m_SerialPorts);

	//
	std::list<SerialPort>::iterator it;
	int i;
	int nPortIdx = -1;
	int nPortNum = -1;

	for (it = m_SerialPorts.begin(), i = 0; it != m_SerialPorts.end(); it++, i++)
	{
		m_wndSerialPort.AddString((*it).m_strName);

		if ((*it).m_nPort == m_sPortNum)
		{
			nPortIdx = i;
			nPortNum = (*it).m_nPort;
		}
	}

	if (nPortIdx < 0)
	{
		m_sPortNum = -1;

		if (m_wndSerialPort.GetCount() > 0)
		{
			m_sPortNum = (*m_SerialPorts.begin()).m_nPort;
			m_wndSerialPort.SetCurSel(0);
		}
	}
	else
	{
		m_wndSerialPort.SetCurSel(nPortIdx);
		m_sPortNum = nPortNum;
	}
}

void CDownloaderDlg::OnChangeLogLevel()
{
	UpdateData(TRUE);

	AfxGetApp()->WriteProfileInt(_T("LogSettings"), _T("Verbo"), m_bVerbo);
}

void CDownloaderDlg::OnCheckDevice()
{
	//
	if (m_State != _READY)
	{
		MessageBeep((UINT)-1);
		return;
	}

	UpdateData(TRUE);
	Connect(TRUE);
}
