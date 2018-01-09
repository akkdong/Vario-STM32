// SerialPortSelectDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "wconsole.h"
#include "SerialPortSelectDlg.h"
#include "afxdialogex.h"

#include <initguid.h>
#include <devguid.h>
#include <setupapi.h>

#pragma comment(lib, "Setupapi.lib")

int EnumSerialPorts(std::list<SerialPort> & ports)
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

CSerial::EDataBits			Map_DataBits[] =
{
	CSerial::EData5,
	CSerial::EData6,
	CSerial::EData7,
	CSerial::EData8,
};

CSerial::EParity			Map_Parity[] =
{
	CSerial::EParNone,
	CSerial::EParOdd,
	CSerial::EParEven,
	CSerial::EParMark,
	CSerial::EParSpace,
};

CSerial::EStopBits			Map_StopBits[] =
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


// CSerialPortSelectDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CSerialPortSelectDlg, CDialogEx)

CSerialPortSelectDlg::CSerialPortSelectDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SELECT_PORT, pParent)
	, m_nPort(0)
	, m_nBaudRate(0)
	, m_nDataBits(0)
	, m_nParity(0)
	, m_nStopBits(0)
	, m_nFlowControl(0)
{
	m_sPortName		= _T("");
	m_sPortNum		= 0;
	m_sBaudRate		= CSerial::EBaud115200;
	m_sDataBits		= CSerial::EData8;
	m_sParity		= CSerial::EParNone;
	m_sStopBits		= CSerial::EStop1;
	m_sFlowControl	= CSerial::EHandshakeOff;
}

CSerialPortSelectDlg::~CSerialPortSelectDlg()
{
}

void CSerialPortSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SERIAL_PORT, m_wndPort);
	DDX_CBIndex(pDX, IDC_SERIAL_PORT, m_nPort);
	DDX_CBIndex(pDX, IDC_SERIAL_BUADRATE, m_nBaudRate);
	DDX_CBIndex(pDX, IDC_SERIAL_DATABITS, m_nDataBits);
	DDX_CBIndex(pDX, IDC_SERIAL_PARITY, m_nParity);
	DDX_CBIndex(pDX, IDC_SERIAL_STOPBITS, m_nStopBits);
	DDX_CBIndex(pDX, IDC_SERIAL_FLOWCONTROL, m_nFlowControl);
}


BEGIN_MESSAGE_MAP(CSerialPortSelectDlg, CDialogEx)
END_MESSAGE_MAP()


// CSerialPortSelectDlg 메시지 처리기입니다.


BOOL CSerialPortSelectDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//
	EnumSerialPorts(m_SerialPorts);

	for (std::list<SerialPort>::iterator it = m_SerialPorts.begin(); it != m_SerialPorts.end(); it++)
		m_wndPort.AddString((*it).m_strName);

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CSerialPortSelectDlg::OnOK()
{
	UpdateData(TRUE);

	CDialogEx::OnOK();
}


void CSerialPortSelectDlg::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnCancel();
}

BOOL CSerialPortSelectDlg::UpdateData(BOOL bSaveAndValidate)
{
	if (!bSaveAndValidate)
	{
		// Port
		if (m_sPortNum)
		{
			int nPortNum = 0;

			for (std::list<SerialPort>::iterator it = m_SerialPorts.begin(); it != m_SerialPorts.end(); it++)
			{
				if (m_sPortNum == (*it).m_nPort)
				{
					m_nPort = nPortNum;
					break;
				}

				nPortNum++;
			}
		}

		// BaudRate
		for (int i = 0; i < sizeof(Map_BaudRate) / sizeof(Map_BaudRate[0]); i++)
		{
			if (Map_BaudRate[i] == m_sBaudRate)
			{
				m_nBaudRate = i;
				break;
			}
		}
		// DataBits
		for (int i = 0; i < sizeof(Map_DataBits) / sizeof(Map_DataBits[0]); i++)
		{
			if (Map_DataBits[i] == m_sDataBits)
			{
				m_nDataBits = i;
				break;
			}
		}
		// Parity
		for (int i = 0; i < sizeof(Map_Parity) / sizeof(Map_Parity[0]); i++)
		{
			if (Map_Parity[i] == m_sParity)
			{
				m_nParity = i;
				break;
			}
		}
		// StopBits
		for (int i = 0; i < sizeof(Map_StopBits) / sizeof(Map_StopBits[0]); i++)
		{
			if (Map_StopBits[i] == m_sStopBits)
			{
				m_nStopBits = i;
				break;
			}
		}
		// Handshake(FlowControl)
		for (int i = 0; i < sizeof(Map_FlowControl) / sizeof(Map_FlowControl[0]); i++)
		{
			if (Map_FlowControl[i] == m_sFlowControl)
			{
				m_nFlowControl = i;
				break;
			}
		}
	}

	BOOL bRet = CDialog::UpdateData(bSaveAndValidate);

	if (bSaveAndValidate)
	{
		// Port
		std::list<SerialPort>::iterator it = m_SerialPorts.begin();
		int index = m_nPort;

		while (index)
		{
			it++;
			index--;
		}

		m_sPortName.Format(_T("\\\\.\\COM%d"), (*it).m_nPort);
		m_sPortNum = (*it).m_nPort;


		// BaudRate
		if (m_nBaudRate >= 0)
			m_sBaudRate = Map_BaudRate[m_nBaudRate];
		// DataBits
		if (m_nDataBits >= 0)
			m_sDataBits = Map_DataBits[m_nDataBits];
		// Parity
		if (m_nParity >= 0)
			m_sParity = Map_Parity[m_nParity];
		// StopBits
		if (m_nStopBits >= 0)
			m_sStopBits = Map_StopBits[m_nStopBits];
		// Handshake(FlowControl)
		if (m_nFlowControl >= 0)
			m_sFlowControl = Map_FlowControl[m_nFlowControl];
	}

	return bRet;
}