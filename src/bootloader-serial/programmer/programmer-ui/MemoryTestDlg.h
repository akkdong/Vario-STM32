#pragma once

#include "HexEdit.h"
#include "Packet.h"

////////////////////////////////////////////////////////////////////////////////////
//

class CDownloaderDlg;


////////////////////////////////////////////////////////////////////////////////////
//

class CMemoryTestDlg : public CDialogEx, protected PacketListener
{
public:
	CMemoryTestDlg(CDownloaderDlg * pParent);
	~CMemoryTestDlg();

	// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MEMORY_TEST };
#endif

protected:
	virtual void		DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual void		OnPacketReceived(PACKET * pPacket);

	BOOL				UpdateData(BOOL bSaveAndValidate = TRUE);

private:
	void				RequestMemory(uint32_t address, uint16_t size);

protected:
	//
	UINT				m_nTimerID;

	CComboBox			m_wndAddress;
	CString				m_strAddress;
	CEdit				m_wndSize;
	CString				m_strSize;
	CHexEdit			m_wndHexEdit;

	//
	uint32_t			m_nAddress;
	uint16_t			m_nReadSize;

	//
	CDownloaderDlg *	m_pDlgMain;

protected:
	virtual BOOL		OnInitDialog();
	virtual void		OnOK();
	virtual void		OnCancel();

	afx_msg void		OnTimer(UINT nIDEvent);
	afx_msg void		OnKillFocusAddress();
	afx_msg void		OnKillFocusSize();

	DECLARE_MESSAGE_MAP()
};

