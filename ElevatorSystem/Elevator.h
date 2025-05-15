#pragma once

#include <QWidget>
#include "ui_Elevator.h"
#include <QSlider>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <queue>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qtimer.h>

enum class ElevatorState {
	Idle,
	Up,
	Down,
	Opening, // 新增状态
	Open,
	Closing
};

class Elevator : public QWidget
{
	Q_OBJECT

public:
	Elevator(int elevator_id, int floor_cnt, QLabel* elevator_floor, QWidget *parent = nullptr);
	~Elevator() {}
public:
	void Init(); // 初始化
	void AddTargetFloor(int floor);
	ElevatorState GetState() const { return state; }
	int GetCurrentFloor() const { return current_floor; }
	const std::queue<int>& GetTargetFloors() const { return target_floors; }
	void InsertTargetFloor(int floor);
private:
	void InitParams(); // 初始化参数
	void InitWidget(); // 初始化控件
	void MoveToNextFloor();
	void UpdateDisplay();
	QPushButton* CreateDoorButton(const QString& text, const QString& color);
	QGroupBox* CreateStatusGroup();
	bool RequestExists(int floor);
private slots:
	void HandleOpenDoor();
	void HandleCloseDoor();
private:
	Ui::ElevatorClass ui;
	int elevator_id; // 电梯ID
	int floor_cnt; // 总楼层数
	int current_floor; // 当前楼层
	std::queue<int> target_floors; // 上行请求队列
	ElevatorState state; // 电梯状态
	QLabel* elevator_floor; // 电梯所在楼层
	bool is_door_open;
	QMap<int, QPushButton*> floorButtons; // 新增：楼层号到按钮的映射

//定时器管理
private:
	QTimer* m_openTimer = nullptr;
	QTimer* m_stayOpenTimer = nullptr;
	QTimer* m_closeTimer = nullptr;
	void ClearAllTimers();
};
