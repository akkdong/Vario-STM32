#include "stdafx.h"
#include "resource.h"
#include "MemoryTestDlg.h"
#include "DownloaderDlg.h"


////////////////////////////////////////////////////////////////////////////////////
//

CMemoryTestDlg::CMemoryTestDlg(CDownloaderDlg * pParent)
	: CDialogEx(IDD_MEMORY_TEST, pParent)
	, m_pDlgMain(pParent)
	, m_nAddress(0x08003000)
	, m_nReadSize(0x0400)
	, m_nTimerID(0)
{
}


CMemoryTestDlg::~CMemoryTestDlg()
{
}


void CMemoryTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_ADDRESS, m_wndAddress);
	DDX_Text(pDX, IDC_ADDRESS, m_strAddress);
	DDX_Control(pDX, IDC_MEMORY_SIZE, m_wndSize);
	DDX_Text(pDX, IDC_MEMORY_SIZE, m_strSize);
	DDX_Control(pDX, IDC_HEXEDIT, m_wndHexEdit);
}


BEGIN_MESSAGE_MAP(CMemoryTestDlg, CDialogEx)
	ON_WM_TIMER()
	ON_CBN_KILLFOCUS(IDC_ADDRESS, &CMemoryTestDlg::OnKillFocusAddress)
	ON_EN_KILLFOCUS(IDC_MEMORY_SIZE, &CMemoryTestDlg::OnKillFocusSize)
END_MESSAGE_MAP()


BOOL CMemoryTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//
	UpdateData(FALSE);

	//
	m_wndAddress.AddString(m_strAddress);
	m_wndHexEdit.SetReadOnly(TRUE);

	// request memory
	RequestMemory(m_nAddress, m_nReadSize);

	return TRUE;
}

void CMemoryTestDlg::OnOK()
{
	UpdateData();

	RequestMemory(m_nAddress, m_nReadSize);
}

void CMemoryTestDlg::OnCancel()
{
	CDialogEx::OnCancel();
}

void CMemoryTestDlg::OnPacketReceived(PACKET * pPacket)
{
	if (pPacket->code == DCODE_DUMP_MEM)
	{
		m_wndHexEdit.SetBaseAddress(m_nAddress);
		m_wndHexEdit.SetData(pPacket->payloadLen - 4, pPacket->d.data);
	}
}

void CMemoryTestDlg::OnTimer(UINT nEventID)
{
	CDialogEx::OnTimer(nEventID);
}

BOOL CMemoryTestDlg::UpdateData(BOOL bSaveAndValidate)
{
	if (!bSaveAndValidate)
	{
		// Local variables -> control variables
		m_strAddress.Format(_T("0x%08X"), m_nAddress);
		m_strSize.Format(_T("0x%04X"), m_nReadSize);
	}

	BOOL bRet = CDialogEx::UpdateData(bSaveAndValidate);

	if (bSaveAndValidate)
	{
		// Control variables -> local variables
	}

	return bRet;
}

void CMemoryTestDlg::RequestMemory(uint32_t address, uint16_t size)
{
	CommandMaker maker;

	maker.start(HCODE_READ);
	maker.push_u32(address);
	maker.push_u16(size);
	maker.finish();

	m_pDlgMain->SendCommand(maker.get_data(), maker.get_size());
}


void CMemoryTestDlg::OnKillFocusAddress()
{
	CString str;
	uint32_t addr;
	TCHAR * end = NULL;

	//
	m_wndAddress.GetWindowTextA(str);

	//
	if (_tcsncmp(str, _T("0x"), 2) == 0)
		addr = _tcstol((LPCTSTR)str + 2, &end, 16);
	else
		addr = _tcstol((LPCTSTR)str, &end, 10);

	// ??? range check

	if (m_nAddress != addr)
	{
		m_nAddress = addr;
		UpdateData(FALSE);
	}
}

void CMemoryTestDlg::OnKillFocusSize()
{
	CString str;
	uint32_t size;
	TCHAR * end = NULL;

	//
	m_wndSize.GetWindowTextA(str);

	//
	if (_tcsncmp(str, _T("0x"), 2) == 0)
		size = _tcstol((LPCTSTR)str + 2, &end, 16);
	else
		size = _tcstol((LPCTSTR)str, &end, 10);
	
	//
	if (size > 0x400)
	{
		AfxMessageBox("reading size limit: 1024(0x400)", MB_ICONEXCLAMATION);
		UpdateData(FALSE);
	}
	else if (m_nReadSize != size)
	{
		m_nReadSize = size;
		UpdateData(FALSE);
	}
}
