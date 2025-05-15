#pragma once
enum class Direction {
	Up,
	Down,
	None
};

enum class ElevatorState {
	Idle,
	Up,
	Down,
	Opening,
	Open,
	Closing,
	Warning
};