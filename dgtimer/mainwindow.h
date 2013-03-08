#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTime>
#include <chrono>
namespace Ui {
	class DG_Timer;
}

class MainWindow : public QMainWindow {
	Q_OBJECT
	enum class State {
		IDLE,		//!< 起動直後
		COUNTING,	//!< カウントダウン中
		COUNTEND	//!< カウント終了
	};
	State			_state;
	Ui::DG_Timer* _ui;
	QTimer*			_timer;

	using Clock = std::chrono::system_clock;
	using TPoint = Clock::time_point;
	using TDur = Clock::duration;
	TPoint			_beginTime, _dstTime;
	int				_durSec;

	void _swState(State st);
	void _onEnter(State st);
	void _onExit(State st);
	void _setDuration(TDur dur);
	void _setDuration(QTime tm);

	public:
		explicit MainWindow(QWidget *parent = 0);
		~MainWindow();

	public slots:
		//! start/stop兼用ボタンを押した時
		void pushTButton();
		void onUpdate();
		void set5min();
		void set25min();
		void set45min();
};

#endif // MAINWINDOW_H
