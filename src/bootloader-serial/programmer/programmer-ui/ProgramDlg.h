#pragma once

#include "Packet.h"
#include "Flash.h"


////////////////////////////////////////////////////////////////////////////////////
//

class CDownloaderDlg;


////////////////////////////////////////////////////////////////////////////////////
//

class CProgramDlg : public CDialogEx, protected PacketListener
{
public:
	CProgramDlg(CDownloaderDlg * pParent);
	~CProgramDlg();

	// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROGRAM };
#endif

	//
	enum State
	{
		_READY,
		_RUN,
		_ERASE,
		_PROGRAM,
		_VERIFY,
		_DONE
	};

protected:
	virtual void		DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual void		OnPacketReceived(PACKET * pPacket);

	BOOL				UpdateData(BOOL bSaveAndValidate = TRUE);

private:
	void				RequestErase(uint32_t address);
	void				RequestProgram(uint32_t address);
	void				RequestVerify(uint32_t address);
	void				RequestRun();

	BOOL				FileExist(LPCTSTR lpszFile, uint32_t * pSize = NULL);
	BOOL				LoadFile(LPCTSTR lpszFile);

	void				EnableControls();
	void				Cleanup();

protected:
	//
	CString				m_strFilePath;
	BOOL				m_bVerifyProgram;
	BOOL				m_bRunAfterProgram;

	//
	CDownloaderDlg *	m_pDlgMain;

	//
	UINT				m_nTimerID;

	//
	State				m_nState;

	uint8_t	*			m_pFirmware;
	uint32_t			m_nFileSize;		// actual file size
	uint32_t			m_nProgramSize;		// file size + page padding

	uint32_t			m_nProgramStart;
	uint32_t			m_nProgramEnd;

	uint32_t			m_nTotalPage;		// program size / PAGE_SIZE
	uint32_t			m_nActivePage;		// 0 ~ m_nTotalPage
	uint32_t			m_nActiveSubPage;	// 0 ~ (PAGE_SIZE / PROGRAM_SIZE)



protected:
	virtual BOOL		OnInitDialog();
	virtual void		OnOK();
	virtual void		OnCancel();

	afx_msg void		OnTimer(UINT nIDEvent);
	afx_msg void		OnBrowse();
	afx_msg void		OnStart();
	afx_msg void		OnExecute();
	afx_msg LRESULT		OnError(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()
};

