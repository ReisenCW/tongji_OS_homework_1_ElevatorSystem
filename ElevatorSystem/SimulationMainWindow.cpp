#include "SimulationMainWindow.h"
#include <ElevatorDisplayWindow.h>
#include <QButtonGroup>
#include <QMessageBox>
#include <QDebug>
#include <climits>

SimulationMainWindow::SimulationMainWindow(ElevatorSystem* elevatorSystem, QWidget* parent)
    : QWidget(parent, Qt::Window), elevatorSystem(elevatorSystem)
{
    ui.setupUi(this);
    Init(elevatorSystem->GetElevatorCount(), elevatorSystem->GetFloorCount());
}

void SimulationMainWindow::Init(int elevator_count, int floor_count)
{
    setWindowTitle("电梯模拟器 Elevator Simulator");
    setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
    CaculateWindowSize(elevator_count, floor_count);
    this->setFixedSize(window_width, window_height);
    move(1200, 300);

    floorButtonStates.resize(floor_count);
    for (auto& state : floorButtonStates) {
        state.upPressed = false;
        state.downPressed = false;
        state.upDirection = Direction::None;
        state.downDirection = Direction::None;
    }
    InitWidget();
    CreateElevatorWinodws();
}

void SimulationMainWindow::InitWidget()
{
    QScrollArea* mainScrollArea = new QScrollArea(this);
    QWidget* containerWidget = new QWidget(mainScrollArea);
    QVBoxLayout* containerLayout = new QVBoxLayout(containerWidget);
    containerLayout->setAlignment(Qt::AlignTop);

    stop_simulation_btn = new QPushButton("停止模拟", this);
    stop_simulation_btn->setGeometry(window_width / 2 - 50, window_height - 50, 100, 40);
    connect(stop_simulation_btn, &QPushButton::clicked, this, &SimulationMainWindow::close);
    connect(this, &SimulationMainWindow::windowClosed, elevatorSystem, &ElevatorSystem::HandleSimulationClosed);

    // 电梯楼层标签
    QWidget* elevatorWidget = new QWidget(containerWidget);
    QGridLayout* elevatorLayout = new QGridLayout(elevatorWidget);
    elevatorLayout->setSpacing(10);
    elevator_floor_labels = new QLabel[elevatorSystem->GetElevatorCount()];
    for (int i = 0; i < elevatorSystem->GetElevatorCount(); ++i) {
        int row = i / 5;
        int col = i % 5;

        QLabel* elevator_label = new QLabel(QString("电梯 %1").arg(i + 1), elevatorWidget);
        elevator_label->setFixedSize(60, 30);
        elevator_label->setAlignment(Qt::AlignCenter);
        elevator_label->setStyleSheet("background-color: lightblue; border: 1px solid black;");
        elevatorLayout->addWidget(elevator_label, row * 2, col);

        elevator_floor_labels[i].setParent(elevatorWidget);
        elevator_floor_labels[i].setFixedSize(60, 30);
        elevator_floor_labels[i].setAlignment(Qt::AlignCenter);
        elevator_floor_labels[i].setStyleSheet("background-color: lightgreen; border: 1px solid black; color: red; border-radius: 10px;");
        elevator_floor_labels[i].setText(QString::number(1));
        elevatorLayout->addWidget(&elevator_floor_labels[i], row * 2 + 1, col);
    }

    // 楼层按钮
    QWidget* floorWidget = new QWidget(containerWidget);
    QGridLayout* floorLayout = new QGridLayout(floorWidget);
    floorLayout->setSpacing(10);
    for (int i = 0; i < elevatorSystem->GetFloorCount(); ++i) {
        int row = i / 5;
        int col = i % 5;

        QLabel* floor_label = new QLabel(QString("楼层 %1").arg(i + 1), floorWidget);
        floor_label->setFixedSize(60, 30);
        floor_label->setAlignment(Qt::AlignCenter);
        floor_label->setStyleSheet("background-color: lightblue; border: 1px solid black;");
        floorLayout->addWidget(floor_label, row * 2, col);

        QWidget* buttonContainer = new QWidget(floorWidget);
        QHBoxLayout* buttonLayout = new QHBoxLayout(buttonContainer);
        buttonLayout->setContentsMargins(0, 0, 0, 0);
        buttonLayout->setSpacing(3);

        QPushButton* up_button;
		if (i != elevatorSystem->GetFloorCount() - 1) {
			up_button = new QPushButton("↑", buttonContainer);
			up_button->setObjectName(QString("up_%1").arg(i));
			up_button->setFixedSize(30, 20);
			up_button->setStyleSheet(
				"QPushButton {"
				"   background-color: white;"
				"   border: 1px solid #666;"
				"   border-radius: 10px;"
				"}"
				"QPushButton:disabled { background-color: red; }"
			);
			connect(up_button, &QPushButton::clicked, this, [=]() {
				if (!HasElevatorStoppedAtFloor(i)) {
					up_button->setDisabled(true);
                    floorButtonStates[i].upPressed = true;
                    floorButtonStates[i].upDirection = Direction::Up;
				}
                AssignExternalRequests(i, Direction::Up);
			});
			buttonLayout->addWidget(up_button);
		}
        
        QPushButton* down_button;
        if (i != 0) {
            down_button = new QPushButton("↓", buttonContainer);
            down_button->setObjectName(QString("down_%1").arg(i));
            down_button->setFixedSize(30, 20);
            down_button->setStyleSheet(
                "QPushButton {"
                "   background-color: white;"
                "   border: 1px solid #666;"
                "   border-radius: 10px;"
                "}"
                "QPushButton:disabled { background-color: red; }"
            );
            connect(down_button, &QPushButton::clicked, this, [=]() {
                if (!HasElevatorStoppedAtFloor(i)) {
                    down_button->setDisabled(true);
                    floorButtonStates[i].downPressed = true;
                    floorButtonStates[i].downDirection = Direction::Down;
                }
                AssignExternalRequests(i, Direction::Down);
            });
            buttonLayout->addWidget(down_button);
        }
        

        buttonContainer->setLayout(buttonLayout);
        floorLayout->addWidget(buttonContainer, row * 2 + 1, col);
    }
    floorLayout->setVerticalSpacing(5);

    containerLayout->addWidget(elevatorWidget);
    containerLayout->addWidget(floorWidget);

    mainScrollArea->setWidget(containerWidget);
    mainScrollArea->setWidgetResizable(true);
    mainScrollArea->setGeometry(0, 0, window_width, window_height - 60);

    connect(this, &SimulationMainWindow::windowClosed, elevatorSystem, &ElevatorSystem::HandleSimulationClosed);
}

void SimulationMainWindow::CreateElevatorWinodws()
{
    ElevatorDisplayWindow* elevatorWindow = new ElevatorDisplayWindow(
        elevatorSystem->GetElevatorCount(),
        elevatorSystem->GetFloorCount(),
        elevator_floor_labels,
        this
    );
    for (auto& elevator : elevators) {
        connect(elevator, &Elevator::FloorArrived, this, &SimulationMainWindow::HandleFloorArrived);
        connect(elevator, &Elevator::AlarmTriggered, this, [=](int id) { // 监听报警
            QMessageBox::warning(
                this,
                "电梯报警",
                QString("电梯 %1 触发紧急报警！").arg(id)
            );
        });
    }
    connect(this, &SimulationMainWindow::windowClosed, elevatorWindow, &ElevatorDisplayWindow::HandleSimulationClosed);
    elevatorWindow->show();
}

void SimulationMainWindow::CaculateWindowSize(int elevator_count, int floor_count)
{
    int elevator_rows = elevator_count / 5;
    if (elevator_count % 5 != 0) {
        elevator_rows++;
    }
    int floor_rows = (floor_count / 5) * 2;
    if (floor_count % 5 != 0) {
        floor_rows += 2;
    }
    window_width = 20 + 5 * 80;
    window_height = 100 + elevator_rows * 80 + floor_rows * 40;
    window_height = std::min(window_height, 500);
    setMinimumSize(window_width, window_height);
}

void SimulationMainWindow::SetElevatorFloor(int id, int floor)
{
    if (id < 1 || id > elevatorSystem->GetElevatorCount()) {
        qDebug() << "电梯ID不合法";
        return;
    }
    elevator_floor_labels[id - 1].setText(QString::number(floor));
    elevator_floor_labels[id - 1].repaint();
}

void SimulationMainWindow::AssignExternalRequests(int floor, Direction dir)
{
    Elevator* best = nullptr;
    int min_distance = INT_MAX;

    // 1. 优先空闲电梯
    for (auto& elevator : elevators) {
        if (elevator->GetState() == ElevatorState::Idle) {
            int dist = abs(elevator->GetCurrentFloor() - floor);
            if (dist < min_distance) {
				qDebug() << "电梯" << elevator->GetElevatorID() << "空闲，距离" << dist;
                min_distance = dist;
                best = elevator;
            }
        }
    }
    // 2. 否则找同方向顺路电梯
    if (!best) {
        int dist = INT_MAX;
        for (auto& elevator : elevators) {
            if (elevator->GetState() == ElevatorState::Up && dir == Direction::Up &&
                elevator->GetCurrentFloor() <= floor) {
				int d = abs(elevator->GetCurrentFloor() - floor);
				if (d < dist) {
					dist = d;
					best = elevator;
				}
            }
            if (elevator->GetState() == ElevatorState::Down && dir == Direction::Down &&
                elevator->GetCurrentFloor() >= floor) {
				int d = abs(elevator->GetCurrentFloor() - floor);
                if (d < dist) {
                    dist = d;
                    best = elevator;
                }
            }
        }
    }
    // 3. 否则找最合适的
    if (!best && !elevators.empty()) {
        best = elevators.front();
    }
    if (best)
        best->AddExternalRequest(floor, dir);
}

void SimulationMainWindow::ScheduleElevator(int request_floor, Direction dir)
{
    AssignExternalRequests(request_floor, dir);
}

void SimulationMainWindow::HandleFloorArrived(int floor, ElevatorState elevatorDirection)
{
    if (floor < 0 || floor >= floorButtonStates.size()) return;

    FloorButtonState& state = floorButtonStates[floor];

    // 检查上按钮是否需要恢复
    if (state.upPressed && state.upDirection == Direction::Up) {
        QPushButton* upBtn = findChild<QPushButton*>(QString("up_%1").arg(floor));
        if (upBtn) {
            upBtn->setDisabled(false);
            state.upPressed = false;
            state.upDirection = Direction::None;
        }
    }
    // 检查下按钮是否需要恢复
    if (state.downPressed && state.downDirection == Direction::Down) {
        QPushButton* downBtn = findChild<QPushButton*>(QString("down_%1").arg(floor));
        if (downBtn) {
            downBtn->setDisabled(false);
            state.downPressed = false;
            state.downDirection = Direction::None;
        }
    }
}

bool SimulationMainWindow::HasElevatorStoppedAtFloor(int floor)
{
    for (auto& elevator : elevators) {
        if (elevator->GetCurrentFloor() == floor &&
            (elevator->GetState() == ElevatorState::Idle ||
                elevator->GetState() == ElevatorState::Open)) {
            return true;
        }
    }
    return false;
}