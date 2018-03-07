#include "stdafx.h"
#include "resource.h"
#include "MemoryDumpDlg.h"
#include "DownloaderDlg.h"
#include "flash.h"

////////////////////////////////////////////////////////////////////////////////////
//

CMemoryDumpDlg::CMemoryDumpDlg(CDownloaderDlg * pParent)
	: CDialogEx(IDD_MEMORY_DUMP, pParent)
	, m_pDlgMain(pParent)
	, m_nAddrRead((uint32_t)-1)
	, m_nTimerID(0)
{
	m_nAddress = AfxGetApp()->GetProfileIntA(_T("DumpSettings"), _T("Address"), ADDRESS_USER_APPLICATION);
	m_nReadSize = AfxGetApp()->GetProfileIntA(_T("DumpSettings"), _T("Size"), 0x1000);
}


CMemoryDumpDlg::~CMemoryDumpDlg()
{
}


void CMemoryDumpDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_ADDRESS, m_wndAddress);
	DDX_Text(pDX, IDC_ADDRESS, m_strAddress);
	DDX_Control(pDX, IDC_MEMORY_SIZE, m_wndSize);
	DDX_Text(pDX, IDC_MEMORY_SIZE, m_strSize);
	DDX_Control(pDX, IDC_HEXEDIT, m_wndHexEdit);
}


BEGIN_MESSAGE_MAP(CMemoryDumpDlg, CDialogEx)
	ON_WM_TIMER()
	ON_CBN_KILLFOCUS(IDC_ADDRESS, &CMemoryDumpDlg::OnKillFocusAddress)
	ON_EN_KILLFOCUS(IDC_MEMORY_SIZE, &CMemoryDumpDlg::OnKillFocusSize)
	ON_BN_CLICKED(IDC_SAVE, &CMemoryDumpDlg::OnSave)
END_MESSAGE_MAP()


BOOL CMemoryDumpDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//
	UpdateData(FALSE);

	//
	m_wndAddress.AddString(m_strAddress);
	m_wndHexEdit.SetReadOnly(TRUE);

	return TRUE;
}

void CMemoryDumpDlg::OnOK()
{
	UpdateData();

	// check address validation
	// ...

	//
	EnableControls(FALSE);

	//
	m_nAddrRead = m_nAddress;
	RequestMemory();
}

void CMemoryDumpDlg::OnCancel()
{
	if (m_nAddrRead == (uint32_t)-1)
		CDialogEx::OnCancel();
}

void CMemoryDumpDlg::OnBPacketReceived(BPacket * pPacket)
{
	if (pPacket->code == DCODE_DUMP_MEM)
	{
		if (m_nAddrRead == m_nAddress)
			m_wndHexEdit.SetData(pPacket->payloadLen - 4, pPacket->d.data);
		else
			m_wndHexEdit.AddData(pPacket->payloadLen - 4, pPacket->d.data);

		//
		m_nAddrRead = m_nAddrRead += pPacket->payloadLen - 4;
		RequestMemory();
	}
	else //if (pPacket->code == DCODE_NACK)
	{
		m_pDlgMain->Log(CDownloaderDlg::_INFO, "Read memory read failed! : error code(0x%04X)", pPacket->e.error);

		CString str;
		str.Format("Read memory failed! : error code(0x%04X)", pPacket->e.error);
		AfxMessageBox(str, MB_ICONSTOP);

		//
		EnableControls(TRUE);
	}
}

void CMemoryDumpDlg::OnTimer(UINT nEventID)
{
	CDialogEx::OnTimer(nEventID);
}

BOOL CMemoryDumpDlg::UpdateData(BOOL bSaveAndValidate)
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

void CMemoryDumpDlg::RequestMemory()
{
	if (m_nAddrRead < m_nAddress + m_nReadSize)
	{
		uint32_t size = min((m_nAddress + m_nReadSize) - m_nAddrRead, PROGRAM_SIZE);
		RequestMemory(m_nAddrRead, size);

		m_pDlgMain->Log(CDownloaderDlg::_INFO, "Read memory at 0x%08X-[0x%04X]", m_nAddrRead, size);
	}
	else
	{
		//
		EnableControls(TRUE);
		m_nAddrRead = (uint32_t)-1;

		m_pDlgMain->Log(CDownloaderDlg::_INFO, "Read Complete!");
		AfxMessageBox("Read Complete!", MB_ICONINFORMATION);
	}
}

void CMemoryDumpDlg::RequestMemory(uint32_t address, uint16_t size)
{
	BPacketMaker maker;

	maker.start(HCODE_READ);
	maker.push_u32(address);
	maker.push_u16(size);
	maker.finish();

	m_pDlgMain->SendCommand(maker.get_data(), maker.get_size());
}

void CMemoryDumpDlg::OnKillFocusAddress()
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

	addr = addr / sizeof(uint32_t) * sizeof(uint32_t);

	// ??? range check

	if (m_nAddress != addr)
	{
		//
		CString strAddr;
		strAddr.Format(_T("0x%08X"), addr);

		if (m_wndAddress.FindString(0, strAddr) == -1)
			m_wndAddress.AddString(strAddr);

		m_nAddress = addr;
		AfxGetApp()->WriteProfileInt(_T("DumpSettings"), _T("Address"), m_nAddress);
		UpdateData(FALSE);
	}
}

void CMemoryDumpDlg::OnKillFocusSize()
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

	size = (size + sizeof(uint32_t) - 1) / sizeof(uint32_t) * sizeof(uint32_t);

	//
	if (size > 0xFFFF)
	{
		CString str;
		str.Format(_T("Reading size limit: %d(0x%04X)"), 0xFFFF, 0xFFFF);
		AfxMessageBox(str, MB_ICONEXCLAMATION);
		UpdateData(FALSE);
	}
	else if (m_nReadSize != size)
	{
		m_nReadSize = size;
		AfxGetApp()->WriteProfileInt(_T("DumpSettings"), _T("Size"), m_nReadSize);
		UpdateData(FALSE);
	}
}

void CMemoryDumpDlg::EnableControls(BOOL bEnable)
{
	GetDlgItem(IDC_ADDRESS)->EnableWindow(bEnable);
	GetDlgItem(IDC_MEMORY_SIZE)->EnableWindow(bEnable);
	GetDlgItem(IDOK)->EnableWindow(bEnable);
}

void CMemoryDumpDlg::OnSave()
{
	if (m_wndHexEdit.GetDataLength() > 0)
	{
		CFileDialog dlg(FALSE, "bin", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "Binary files (*.bin)|*.bin|All files (*.*)|*.*||");

		if (dlg.DoModal() == IDOK)
		{
			CFile file;

			if (file.Open(dlg.GetPathName(), CFile::modeCreate | CFile::modeWrite))
			{
				void * pData = malloc(m_wndHexEdit.GetDataLength());

				if (pData != NULL)
				{
					m_wndHexEdit.GetData(m_wndHexEdit.GetDataLength(), (BYTE *)pData);

					file.Write(pData, m_wndHexEdit.GetDataLength());
					file.Close();

					CString str;
					str.Format("Save image to file: %s", dlg.GetPathName());
					AfxMessageBox(str, MB_ICONINFORMATION);
				}
				else
				{
					AfxMessageBox("Memory allocation failed!", MB_ICONSTOP);
				}
			}
			else
			{
				CString str;
				str.Format("File open failed: %s", dlg.GetPathName());
				AfxMessageBox(str, MB_ICONSTOP);
			}
		}
	}
}
