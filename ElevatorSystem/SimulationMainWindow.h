#pragma once
#include <qwidget.h>
#include "ui_SimulationMainWindow.h"
#include <qpushbutton.h>
#include <qlabel.h>
#include <QScrollArea>
#include <ElevatorSystem.h>
#include <Elevator.h>
#include <vector>

enum class Direction {
	Up,
	Down,
	None
};

struct FloorButtonState {
	bool upPressed = false;      // 上按钮是否被按下
	bool downPressed = false;    // 下按钮是否被按下
	Direction upDirection = Direction::None;  // 上按钮请求方向
	Direction downDirection = Direction::None;// 下按钮请求方向
};

class SimulationMainWindow : public QWidget
{
	Q_OBJECT

public:
	SimulationMainWindow(ElevatorSystem* elevatorSystem, QWidget *parent = nullptr);
	~SimulationMainWindow(){}
private:
	Ui::SimulationMainWindowClass ui;
private:
	void Init(int elevator_count, int floor_count);
	void InitWidget();
	void CreateElevatorWinodws();
	void closeEvent(QCloseEvent* event) {
		emit windowClosed();
		QWidget::closeEvent(event);
	}
	void CaculateWindowSize(int elevator_count, int floor_count);
	void ScheduleElevator(int request_floor, Direction dir);
	void HandleFloorArrived(int floor, ElevatorState elevatorDirection);
	bool HasElevatorStoppedAtFloor(int floor);

signals:
	void windowClosed();
public:
	void SetElevatorFloor(int id, int floor);
	void AddElevator(Elevator* elevator) {
		elevators.push_back(elevator);
	}
private:
	QPushButton* stop_simulation_btn;
	ElevatorSystem* elevatorSystem;
	QLabel* elevator_floor_labels; // 数组指针
	QButtonGroup* elevator_buttons; // 电梯按钮组(上，下)
	std::vector<Elevator*> elevators; // 电梯对象数组
	std::vector<FloorButtonState> floorButtonStates; // 楼层按钮状态数组
private:
	int window_width;
	int window_height;
};
