// HistoryTrafficList.cpp: 实现文件
//

#include "stdafx.h"
#include "TrafficMonitor.h"
#include "HistoryTrafficListDlg.h"
#include "afxdialogex.h"


// CHistoryTrafficListDlgDlg 对话框

IMPLEMENT_DYNAMIC(CHistoryTrafficListDlg, CTabDlg)

CHistoryTrafficListDlg::CHistoryTrafficListDlg(deque<HistoryTraffic>& history_traffics, CWnd* pParent /*=nullptr*/)
	: CTabDlg(IDD_HISTORY_TRAFFIC_LIST_DIALOG, pParent), m_history_traffics(history_traffics)
{

}

CHistoryTrafficListDlg::~CHistoryTrafficListDlg()
{
}


void CHistoryTrafficListDlg::DoDataExchange(CDataExchange* pDX)
{
	CTabDlg::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_HISTORY_INFO_LIST, m_history_list);
}


BEGIN_MESSAGE_MAP(CHistoryTrafficListDlg, CTabDlg)
	ON_WM_INITMENU()
	ON_COMMAND(ID_USE_LINEAR_SCALE, &CHistoryTrafficListDlg::OnUseLinearScale)
	ON_COMMAND(ID_USE_LOG_SCALE, &CHistoryTrafficListDlg::OnUseLogScale)
END_MESSAGE_MAP()


// CHistoryTrafficListDlgDlg 消息处理程序


BOOL CHistoryTrafficListDlg::OnInitDialog()
{
	CTabDlg::OnInitDialog();

	// TODO:  在此添加额外的初始化

	//初始化列表控件
	CRect rect;
	m_history_list.GetClientRect(rect);
	m_history_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);
	int width0, width1;
	width0 = rect.Width() / 3;
	width1 = rect.Width() - 2 * width0 - theApp.DPI(21);
	m_history_list.InsertColumn(0, CCommon::LoadText(IDS_DATE), LVCFMT_LEFT, width0);		//插入第0列
	m_history_list.InsertColumn(1, CCommon::LoadText(IDS_TRAFFIC_USED), LVCFMT_LEFT, width0);		//插入第1列
	m_history_list.InsertColumn(2, CCommon::LoadText(IDS_FIGURE), LVCFMT_LEFT, width1);

	//获取历史流量列表中流量的最大值
	unsigned int max_traffic{};
	for (const auto& traffic : m_history_traffics)
	{
		if (traffic.kBytes > max_traffic)
			max_traffic = traffic.kBytes;
	}
	m_history_list.EnableDrawItemRange();
	m_history_list.SetDrawItemRangeRow(2);
	m_history_list.SetDrawItemRangMargin(theApp.DPI(4));
	m_history_list.SetDrawItemRangInLogScale(theApp.m_cfg_data.m_use_log_scale);

	for (size_t i{}; i<m_history_traffics.size(); i++)
	{
		CString date_str;
		CString k_bytes_str;
		date_str.Format(_T("%.4d/%.2d/%.2d"), m_history_traffics[i].year, m_history_traffics[i].month, m_history_traffics[i].day);

		if (m_history_traffics[i].kBytes < 1024)
			k_bytes_str.Format(_T("%d KB"), m_history_traffics[i].kBytes);
		else if (m_history_traffics[i].kBytes < 1024 * 1024)
			k_bytes_str.Format(_T("%.2f MB"), m_history_traffics[i].kBytes / 1024.0);
		else
			k_bytes_str.Format(_T("%.2f GB"), m_history_traffics[i].kBytes / 1024.0 / 1024.0);

		m_history_list.InsertItem(i, date_str);
		m_history_list.SetItemText(i, 1, k_bytes_str);
		double range = static_cast<double>(m_history_traffics[i].kBytes) * 1000 / max_traffic;
		COLORREF color;
		if (m_history_traffics[i].kBytes < 102400)		//流量小于100MB时绘制蓝色
			color = TRAFFIC_COLOR_BLUE;
		else if (m_history_traffics[i].kBytes < 1024 * 1024)	//流量小于1GB时绘制绿色
			color = TRAFFIC_COLOR_GREEN;
		else if (m_history_traffics[i].kBytes < 10 * 1024 * 1024)	//流量小于10GB时绘制黄色
			color = TRAFFIC_COLOR_YELLOE;
		else		//流量大于10GB时绘制红色
			color = TRAFFIC_COLOR_RED;
		m_history_list.SetDrawItemRangeData(i, range, color);
	}

	m_menu.LoadMenu(IDR_HISTORY_TRAFFIC_MENU);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


BOOL CHistoryTrafficListDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// TODO: 在此添加专用代码和/或调用基类

	LPNMHDR lpnmhdr = (LPNMHDR)lParam;
	if (lpnmhdr->code == NM_RCLICK)
	{
		CPoint point;
		GetCursorPos(&point);//获得光标的位置
		m_history_list.ScreenToClient(&point);//获得list控件在窗口上的坐标
		CWnd* pWnd = m_history_list.ChildWindowFromPoint(point);
		CHeaderCtrl* pHeader = m_history_list.GetHeaderCtrl();//获取列表视图控件的标题控件
		CRect item_rect;
		pHeader->GetItemRect(2, item_rect);		//获取列表标题控件第2列的矩形区域，只有光标在第2列点击时才弹出右键菜单
		if (pWnd && (pWnd->GetSafeHwnd() == pHeader->GetSafeHwnd()) && item_rect.PtInRect(point))
		{
			HDHITTESTINFO info{};
			info.pt = point;
			pHeader->SendMessage(HDM_HITTEST, 0, (LPARAM)&info);
			CMenu * pMenu = m_menu.GetSubMenu(0);
			GetCursorPos(&point);
			pMenu->TrackPopupMenu(TPM_RIGHTBUTTON, point.x, point.y, this);
		}
	}
	return CTabDlg::OnNotify(wParam, lParam, pResult);
}


void CHistoryTrafficListDlg::OnInitMenu(CMenu* pMenu)
{
	CTabDlg::OnInitMenu(pMenu);

	// TODO: 在此处添加消息处理程序代码
	if (theApp.m_cfg_data.m_use_log_scale)
		pMenu->CheckMenuRadioItem(ID_USE_LINEAR_SCALE, ID_USE_LOG_SCALE, ID_USE_LOG_SCALE, MF_BYCOMMAND | MF_CHECKED);
	else
		pMenu->CheckMenuRadioItem(ID_USE_LINEAR_SCALE, ID_USE_LOG_SCALE, ID_USE_LINEAR_SCALE, MF_BYCOMMAND | MF_CHECKED);
}


void CHistoryTrafficListDlg::OnUseLinearScale()
{
	// TODO: 在此添加命令处理程序代码
	theApp.m_cfg_data.m_use_log_scale = false;
	m_history_list.SetDrawItemRangInLogScale(theApp.m_cfg_data.m_use_log_scale);
	theApp.SaveConfig();
}


void CHistoryTrafficListDlg::OnUseLogScale()
{
	// TODO: 在此添加命令处理程序代码
	theApp.m_cfg_data.m_use_log_scale = true;
	m_history_list.SetDrawItemRangInLogScale(theApp.m_cfg_data.m_use_log_scale);
	theApp.SaveConfig();
}
