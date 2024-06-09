#pragma once
#include "pch.h"


class CeresSolver : public ceres::IterationCallback
{
public:
	typedef ceres::Solver::Options SolverOptions;

	virtual ~CeresSolver()
	{
		if (problem)
			delete problem;
	}

	CeresSolver() 
	{
#ifdef _DEBUG
		solverOptions.num_threads = 1;
#else
		solverOptions.num_threads = 14;
#endif
		solverOptions.update_state_every_iteration = true;
		solverOptions.callbacks.push_back(this);
	}

	SolverOptions& getSolverOptions() 
	{ 
		return solverOptions; 
	}
	
	void setNumThread(int num) 
	{
		solverOptions.num_threads = num;
	}

	void setMaxNumIteration(int num)
	{
		solverOptions.max_num_iterations = num;
	}

	void setFunctionTolerance(double tolerance)
	{
		solverOptions.function_tolerance = tolerance;
	}

	ceres::CallbackReturnType operator()(const ceres::IterationSummary& summary) override 
	{
		return ceres::CallbackReturnType::SOLVER_CONTINUE;
	}
	

protected:
	void solve()
	{
		ceres::Solver::Summary summary;

		std::cout << "Solving the problem.\n";
		Solve(solverOptions, problem, &summary);

		std::string result = summary.FullReport();
		std::cout << result << "\n";
	}

	SolverOptions solverOptions;
	const int maxParams = 20;
	ceres::Problem* problem = nullptr;
};