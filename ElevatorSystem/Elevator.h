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
#include <set>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qtimer.h>
#include <vector>
#include <Utilities.h>


class Elevator : public QWidget
{
    Q_OBJECT

public:
    Elevator(int elevator_id, int floor_cnt, QLabel* elevator_floor, QWidget* parent = nullptr);
    ~Elevator() {}
public:
    void Init();
    void AddInternalTarget(int floor); // 电梯内目标
    void AddExternalRequest(int floor, Direction dir); // 电梯外请求
    ElevatorState GetState() const { return state; }
    int GetCurrentFloor() const { return current_floor; }
    int GetElevatorID() const { return elevator_id; }
    void UpdateDisplay();
    void MoveToNextFloor();
    void DecideNextAction();
private:
    void InitParams();
    void InitWidget();
    QPushButton* CreateDoorButton(const QString& text, const QString& color);
    QGroupBox* CreateStatusGroup();
    bool InternalRequestExists(int floor) const;
    bool ExternalRequestExists(int floor, Direction dir) const;
	void ClearAllTimers();
public slots:
    void HandleOpenDoor();
    void HandleCloseDoor();
	void HandleAlarm();

private:
    Ui::ElevatorClass ui;
    int elevator_id;
    int floor_cnt;
    int current_floor;
    ElevatorState state;
    Direction direction;
    QLabel* elevator_floor;
    bool is_door_open;
    bool is_alarm_active;
    std::vector<QPushButton*> floorButtons;

    // 请求管理
    std::set<int> internal_targets;         // 电梯内目标楼层
    std::set<int> external_up_requests;     // 外部上行请求
    std::set<int> external_down_requests;   // 外部下行请求

    // 定时器管理
    QTimer* m_openTimer = nullptr;
    QTimer* m_stayOpenTimer = nullptr;
    QTimer* m_closeTimer = nullptr;

signals:
    void FloorArrived(int floor, ElevatorState direction);
    void AlarmTriggered(int elevator_id);
};