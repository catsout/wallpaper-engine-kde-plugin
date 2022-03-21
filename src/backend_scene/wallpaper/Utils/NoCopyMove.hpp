#pragma once

struct NoCopy {
  protected:
	NoCopy()  = default;
	~NoCopy() = default;

	NoCopy(const NoCopy& rhs) = delete;

	NoCopy& operator=(const NoCopy& rhs) = delete;
};

struct NoMove {
  protected:
	NoMove()  = default;
	~NoMove() = default;

	NoMove(NoMove&& rhs) = delete;

	NoMove& operator=(NoMove&& rhs) = delete;
};
