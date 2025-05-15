#include "SimulationMainWindow.h"
#include <ElevatorDisplayWindow.h>

SimulationMainWindow::SimulationMainWindow(ElevatorSystem* elevatorSystem, QWidget *parent)
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
	move(1200, 300); // 设置窗口初始位置
	InitWidget();
	CreateElevatorWinodws();
}

void SimulationMainWindow::InitWidget()
{
	// 创建滚动区域和容器
	QScrollArea* mainScrollArea = new QScrollArea(this);
	QWidget* containerWidget = new QWidget(mainScrollArea);
	QVBoxLayout* containerLayout = new QVBoxLayout(containerWidget);
	containerLayout->setAlignment(Qt::AlignTop);

	// 停止按钮（直接放在窗口底部，不滚动）
	stop_simulation_btn = new QPushButton("停止模拟", this);
	stop_simulation_btn->setGeometry(window_width / 2 - 50, window_height - 50, 100, 40);
	// 停止模拟后关闭该窗口
	connect(stop_simulation_btn, &QPushButton::clicked, this, &SimulationMainWindow::close);
	// 关闭窗口后发出信号，告诉主窗口重新显示
	connect(this, &SimulationMainWindow::windowClosed, elevatorSystem, &ElevatorSystem::HandleSimulationClosed);

	// 上半部分：所有电梯所在楼层的显示。上面是电梯编号，下面是电梯所在楼层
	// 创建一个label数组来存储电梯所在楼层的标签
	// 动态生成电梯标签
	// 电梯楼层标签部分
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
		elevatorLayout->addWidget(elevator_label, row*2, col);

		elevator_floor_labels[i].setParent(elevatorWidget);
		elevator_floor_labels[i].setFixedSize(60, 30);
		elevator_floor_labels[i].setAlignment(Qt::AlignCenter);
		elevator_floor_labels[i].setStyleSheet("background-color: lightgreen; border: 1px solid black; color: red; border-radius: 10px;");
		elevator_floor_labels[i].setText(QString::number(1));
		elevatorLayout->addWidget(&elevator_floor_labels[i], row*2 + 1, col); // 楼层标签在下一行
	}

	// 下半部分：每个楼层以及上下按钮。第一行楼层编号，第二行是上按钮，第三行是下按钮，一行5个，暂时不写connect
	// 楼层按钮部分
	QWidget* floorWidget = new QWidget(containerWidget);
	QGridLayout* floorLayout = new QGridLayout(floorWidget);
	floorLayout->setSpacing(10);
	// 动态生成楼层按钮
	for (int i = 0; i < elevatorSystem->GetFloorCount(); ++i) {
		int row = i / 5;  // 每组5个楼层，占用两行（标签一行，按钮一行）
		int col = i % 5;

		// 楼层标签
		QLabel* floor_label = new QLabel(QString("楼层 %1").arg(i + 1), floorWidget);
		floor_label->setFixedSize(60, 30);
		floor_label->setAlignment(Qt::AlignCenter);
		floor_label->setStyleSheet("background-color: lightblue; border: 1px solid black;");
		floorLayout->addWidget(floor_label, row * 2, col); // 标签放在偶数行

		// 上下按钮容器（水平布局）
		QWidget* buttonContainer = new QWidget(floorWidget);
		QHBoxLayout* buttonLayout = new QHBoxLayout(buttonContainer);
		buttonLayout->setContentsMargins(0, 0, 0, 0); // 移除边距
		buttonLayout->setSpacing(3); // 按钮间距

		// 上按钮
		QPushButton* up_button = new QPushButton("↑", buttonContainer);
		up_button->setFixedSize(30, 20);
		up_button->setStyleSheet("background-color: lightgreen; border: 1px solid black; border-radius: 10px;");
		buttonLayout->addWidget(up_button);
		connect(up_button, &QPushButton::clicked, this, [=]() {
			ScheduleElevator(i, Direction::Up); // i 是当前楼层
			});
		// 下按钮
		QPushButton* down_button = new QPushButton("↓", buttonContainer);
		down_button->setFixedSize(30, 20);
		down_button->setStyleSheet("background-color: lightcoral; border: 1px solid black; border-radius: 10px;");
		buttonLayout->addWidget(down_button);
		connect(down_button, &QPushButton::clicked, this, [=]() {
			ScheduleElevator(i, Direction::Down);
			});
		// 将按钮容器添加到奇数行
		floorLayout->addWidget(buttonContainer, row * 2 + 1, col);
	}

	// 调整楼层布局的垂直间距
	floorLayout->setVerticalSpacing(5); // 标签和按钮之间的间距

	// 将电梯和楼层部分添加到容器
	containerLayout->addWidget(elevatorWidget);
	containerLayout->addWidget(floorWidget);

	// 配置滚动区域
	mainScrollArea->setWidget(containerWidget);
	mainScrollArea->setWidgetResizable(true);
	mainScrollArea->setGeometry(0, 0, window_width, window_height - 60); // 留出底部按钮空间

	// 信号连接
	connect(this, &SimulationMainWindow::windowClosed, elevatorSystem, &ElevatorSystem::HandleSimulationClosed);
}

void SimulationMainWindow::CreateElevatorWinodws()
{
	// 创建新窗口显示所有电梯
	ElevatorDisplayWindow* elevatorWindow = new ElevatorDisplayWindow(
		elevatorSystem->GetElevatorCount(),
		elevatorSystem->GetFloorCount(),
		elevator_floor_labels, // 传递电梯楼层标签
		this
	);
	connect(this, &SimulationMainWindow::windowClosed, elevatorWindow, &ElevatorDisplayWindow::HandleSimulationClosed);
	elevatorWindow->show();
}


void SimulationMainWindow::CaculateWindowSize(int elevator_count, int floor_count)
{
	// 计算电梯部分行数
	int elevator_rows = elevator_count / 5;
	if (elevator_count % 5 != 0) {
		elevator_rows++;
	}

	// 计算楼层部分行数（每个楼层组占用两行）
	int floor_rows = (floor_count / 5) * 2;
	if (floor_count % 5 != 0) {
		floor_rows += 2; // 剩余楼层占用两行
	}

	// 调整窗口高度
	window_width = 20 + 5 * 80; // 宽度保持不变
	window_height = 100 + elevator_rows * 80 + floor_rows * 40; // 行高按实际内容调整

	// 窗口高度不能太高
	window_height = std::min(window_height, 500); // 设置最大高度为500

	setMinimumSize(window_width, window_height);
}

void SimulationMainWindow::SetElevatorFloor(int id, int floor)
{
	// 设置电梯所在楼层
	if (id < 1 || id > elevatorSystem->GetElevatorCount()) {
		qDebug() << "电梯ID不合法";
		return;
	}
	elevator_floor_labels[id - 1].setText(QString::number(floor));
	elevator_floor_labels[id - 1].repaint(); // 强制重绘
}

void SimulationMainWindow::ScheduleElevator(int request_floor, Direction dir) {
	int min_cost = INT_MAX;
	Elevator* selected = nullptr;

	for (auto& elevator : elevators) {
		// 计算总移动成本
		int cost = 0;
		int last_floor = elevator->GetCurrentFloor();
		auto targets = elevator->GetTargetFloors(); // 获取目标队列副本

		// 现有任务移动距离
		while (!targets.empty()) {
			int next = targets.front();
			cost += abs(next - last_floor);
			last_floor = next;
			targets.pop();
		}

		// 加上从最后目标到请求楼层的距离
		cost += abs(request_floor - last_floor);

		// 选择成本最小的电梯
		if (cost < min_cost) {
			min_cost = cost;
			selected = elevator;
		}
	}

	if (selected) {
		if (selected->GetState() == ElevatorState::Idle) {
			// 若电梯空闲，触发移动
			selected->AddTargetFloor(request_floor);
		}
	}
	else {
		QMessageBox::warning(this, "警告", "无可用电梯");
	}
}