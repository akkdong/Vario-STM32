#include "stdafx.h"
#include <strsafe.h>
#include "programmer-ui.h"
#include "DownloaderDlg.h"
#include "SerialPortSelectDlg.h"
#include "ProgramDlg.h"
#include "UploadDlg.h"
#include "MemoryTestDlg.h"

#define TIMER_IDENTIFY			(0x5424)


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
	, m_nPortNum(0)
	, m_nBaudRate(CSerial::EBaud57600)
	, m_nDataBits(CSerial::EData8)
	, m_nParity(CSerial::EParNone)
	, m_nStopBits(CSerial::EStop1)
	, m_strDevID("")
	, m_strFWVer("")
	, m_nDevID(0)
	, m_nFWVer(0)
	, m_pPacketListener(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CDownloaderDlg::~CDownloaderDlg()
{
}


void CDownloaderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_DEV_ID, m_strDevID);
	DDX_Text(pDX, IDC_FW_VER, m_strFWVer);
	DDX_Control(pDX, IDC_REPORT_LIST, m_wndReport);
}


BEGIN_MESSAGE_MAP(CDownloaderDlg, CDialogEx)
	ON_WM_TIMER()
	ON_REGISTERED_MESSAGE(CSerialWnd::mg_nDefaultComMsg, OnSerialMessage)
	ON_BN_CLICKED(IDC_CONNECT, OnConnect)
	ON_BN_CLICKED(IDC_DOWNLOAD, OnDownload)
	ON_BN_CLICKED(IDC_UPLOAD, OnUpload)
	ON_BN_CLICKED(IDC_MEMORY, OnMemoryTest)
	ON_BN_CLICKED(IDC_TEST, OnTest)
	ON_BN_CLICKED(IDC_TEST2, OnTest2)
	ON_COMMAND(IDC_IDENTIFY, OnIdentify)
END_MESSAGE_MAP()


BOOL CDownloaderDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	//
	UpdateData(FALSE);

	//PostMessage(WM_COMMAND, MAKEWPARAM(IDC_CONNECT, BN_CLICKED), (LPARAM)GetDlgItem(IDC_CONNECT)->GetSafeHwnd());

	return TRUE;
}

void CDownloaderDlg::OnOK()
{
}

void CDownloaderDlg::OnCancel()
{
	CDialogEx::OnCancel();
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
	if (nIDEvent == m_nTimerID)
	{
		if ((m_subState % 4) == 0)
			SendResetRequest();
		//else if ((m_subState % 4) == 1)
		//	skip(wait)
		else if ((m_subState % 4) == 2)
			SendIdentify();
		//else if ((m_subState % 4) == 3)
		//	skip(wait)

		m_subState += 1;

		if (m_subState > 4 * 5)
		{
			// no reponse!!
			::MessageBeep(-1);

			KillTimer(m_nTimerID);
			m_nTimerID = 0;

			Log("device not reponse!!");
			CloseSerial();

			m_nDevID = 0;
			m_nFWVer = 0;

			UpdateData(FALSE);
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
			if (m_Parser.push(ch))
			{
				//PACKET * pPacket = new PACKET;
				PACKET packet;
				PACKET * pPacket = &packet;
				m_Parser.getPacket(pPacket);

				Log("recieve %02X: %d bytes", pPacket->code, pPacket->payloadLen);
				Route(pPacket);
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
	//if (!m_bConnected)
	if (m_State == _READY)
	{
		CSerialPortSelectDlg dlg;

		dlg.m_sPortNum = m_nPortNum;
		dlg.m_sBaudRate = m_nBaudRate;
		dlg.m_sDataBits = m_nDataBits;
		dlg.m_sParity = m_nParity;
		dlg.m_sStopBits = m_nStopBits;

		if (dlg.DoModal() == IDOK)
		{
			//
			m_nPortNum = dlg.m_sPortNum;
			m_nBaudRate = dlg.m_sBaudRate;
			m_nDataBits = dlg.m_sDataBits;
			m_nParity = dlg.m_sParity;
			m_nStopBits = dlg.m_sStopBits;

			//
			OpenSerial(dlg.m_sPortName, dlg.m_sBaudRate, dlg.m_sDataBits, dlg.m_sParity, dlg.m_sStopBits, dlg.m_sFlowControl);
		}
	}
	else
	{
		//
		CloseSerial();

		//
		m_nDevID = 0;
		m_nFWVer = 0;

		UpdateData(FALSE);
	}

	UpdateTitle();
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

		switch (m_nBaudRate)
		{
		case CSerial::EBaud9600: b = 9600;		break;
		case CSerial::EBaud14400: b = 14400;	break;
		case CSerial::EBaud19200: b = 19200;	break;
		case CSerial::EBaud38400: b = 38400;	break;
		case CSerial::EBaud56000: b = 56000;	break;
		case CSerial::EBaud57600: b = 57600;	break;
		case CSerial::EBaud115200: b = 115200;	break;
		}

		switch (m_nDataBits)
		{
		case CSerial::EData5: d = 5; break;
		case CSerial::EData6: d = 6; break;
		case CSerial::EData7: d = 7; break;
		case CSerial::EData8: d = 8; break;
		}

		switch (m_nParity)
		{
		case CSerial::EParNone: p = 'N'; break;
		case CSerial::EParOdd: p = 'O'; break;
		case CSerial::EParEven: p = 'E'; break;
		case CSerial::EParMark: p = 'M'; break;
		case CSerial::EParSpace: p = 'S'; break;
		}

		switch (m_nStopBits)
		{
		case CSerial::EStop1: s = "1";	break;
		case CSerial::EStop1_5: s = "1.5"; break;
		case CSerial::EStop2: s = "2";	break;

		}

		strTitle.Format(_T("STM32F1 Programmer - COM%d:%d,%d,%c,%s"), m_nPortNum, b, d, p, s);
	}
	else
	{
		strTitle.Format("STM32F1 Programmer - Unconnected!");
	}

	//
	//GetDlgItem(IDC_CONNECT)->SetWindowTextA((m_bConnected ? _T("Disconnect") : _T("Connect")));
	GetDlgItem(IDC_CONNECT)->SetWindowTextA((m_State != _READY ? _T("Disconnect") : _T("Connect")));
	//
	SetWindowText(strTitle);
}


BOOL CDownloaderDlg::UpdateData(BOOL bSaveAndValidate)
{
	if (!bSaveAndValidate)
	{
		//
		UpdateTitle();

		m_strDevID.Format("%04X", m_nDevID);
		m_strFWVer.Format("%04X", m_nFWVer);
	}

	BOOL bRet = CDialog::UpdateData(bSaveAndValidate);

	if (bSaveAndValidate)
	{
		//
	}

	return bRet;
}

void CDownloaderDlg::OpenSerial(LPCTSTR lpszDevice, CSerial::EBaudrate nBaudRate, CSerial::EDataBits nDataBits, CSerial::EParity nParity, CSerial::EStopBits nStopBits, CSerial::EHandshake nHandshake)
{
	CWaitCursor wait;
	LONG lResult;

	if ((lResult = m_Serial.Open(lpszDevice, m_hWnd)) == ERROR_SUCCESS)
	{
		//
		m_Serial.Setup(nBaudRate, nDataBits, nParity, nStopBits);
		m_Serial.SetupHandshaking(nHandshake);
		m_Serial.SetupReadTimeouts(CSerial::EReadTimeoutNonblocking);

		Log("connected...");

		//
		m_State = _IDENTIFY;
		m_subState = 0;

		// set timer to identify device
		//   on timer -> first send reset request to user application
		//               and send identify command to bootloader after 500ms 
		m_nTimerID = SetTimer(TIMER_IDENTIFY, 500, NULL);
	}
	else
	{
		ShowLastError(_T("Serial connection"), lResult);
		Log("connection failed: %X", lResult);
	}
}

void CDownloaderDlg::CloseSerial()
{
	m_Serial.Close();

	if (m_nTimerID)
	{
		KillTimer(m_nTimerID);
		m_nTimerID = 0;
	}

	m_State = _READY;

	Log("disconnected!!");
}

void CDownloaderDlg::OnDownload()
{
	if (m_State == _RUN)
	{
		CProgramDlg dlg(this);

		m_pPacketListener = (PacketListener *)&dlg;
		INT_PTR ret = dlg.DoModal();
		m_pPacketListener = (PacketListener *)this;

		if (ret == IDYES)
			OnConnect();
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

		m_pPacketListener = (PacketListener *)&dlg;
		INT_PTR ret = dlg.DoModal();
		m_pPacketListener = (PacketListener *)this;
	}
	else
	{
		::MessageBeep((UINT)-1);
	}
}

void CDownloaderDlg::OnMemoryTest()
{
	if (m_State == _RUN)
	{
		CMemoryTestDlg dlg(this);

		m_pPacketListener = (PacketListener *)&dlg;
		INT_PTR ret = dlg.DoModal();
		m_pPacketListener = (PacketListener *)this;
	}
	else
	{
		::MessageBeep((UINT)-1);
	}
}

void CDownloaderDlg::SendResetRequest()
{
	if (m_State == _IDENTIFY)
	{
		// #RB\r\n
		m_Serial.Write("#RB\r\n", 5);

		Log("send reset request: #%d", m_subState / 4 + 1);
	}
}

void CDownloaderDlg::SendIdentify()
{
	if (m_State != _READY)
	{
		CommandMaker maker;

		maker.start(HCODE_IDENTIFY);
		maker.finish();

		m_Serial.Write(maker.get_data(), maker.get_size());

		Log("send identify command: #%d", m_subState / 4 + 1);
	}
}

void CDownloaderDlg::OnIdentify()
{
	// send reset request to user application
	SendResetRequest();
	Sleep(500);
	// send identify to bootloader
	SendIdentify();
}

void CDownloaderDlg::Log(LPCTSTR format, ...)
{
	TCHAR buf[1024];
	va_list	va;

	va_start(va, format);
	vsprintf_s(buf, format, va);
	va_end(va);

	m_wndReport.InsertString(0, buf);
	m_wndReport.SetCurSel(0);
}

void CDownloaderDlg::Route(PACKET * pPacket)
{
	if (m_pPacketListener)
		m_pPacketListener->OnPacketReceived(pPacket);
	else
		this->OnPacketReceived(pPacket);

	//delete pPacket;
}

void CDownloaderDlg::OnPacketReceived(PACKET * pPacket)
{
	TRACE("> CODE : %02X\n", pPacket->code);
	switch (pPacket->code)
	{
	case DCODE_IDENTIFY:
		TRACE("    Dev-ID: %04X\n", pPacket->i.devId);
		TRACE("    F/W ver: %04X\n", pPacket->i.fwVer);

		if (m_State == _IDENTIFY)
		{
			m_State = _RUN;

			m_nDevID = pPacket->i.devId;
			m_nFWVer = pPacket->i.fwVer;

			KillTimer(m_nTimerID);
			UpdateData(FALSE);

			Log("device identified");
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

void CDownloaderDlg::SendCommand(uint8_t * pData, uint16_t nDataLen)
{
	//
	m_Serial.Write(pData, nDataLen);
	//
	Log("send %02X: %d byte(s)", pData[1], nDataLen);
}

void CDownloaderDlg::OnTest()
{
	m_wndReport.ResetContent();
}

void CDownloaderDlg::OnTest2()
{
	if (m_State == _RUN)
	{
		CommandMaker maker;

		maker.start(HCODE_START);
		maker.finish();

		this->SendCommand(maker.get_data(), maker.get_size());

		OnConnect();
	}
}
