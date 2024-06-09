#include "pch.h"
#include <iostream>
#include <fstream>
#include "AppearanceSolver.h"


int main()
{
	const int width = 1024;
	const int height = 1024;

	try {
		AppearanceSolver solver(width, height);
		solver.setNumThread(4);

		solver.setInputDirectory("Data/example/");
		solver.setDomain(0, 0, 1024, 1024);
		
		solver.setViewWeightMin(0.001);
		solver.setViewWeightBias(8.0);
		solver.setZeroRadius(4);

		solver.setNormalMode(NormalOptMode::heightmap2018);
		solver.setDifferenceMode(DifferenceMode::forward2);

		solver.setParamSpace(
			ParamSpace::param_diffuse
			//| ParamSpace::param_specular
			//| ParamSpace::param_roughness
			//| ParamSpace::param_normal
		);

		solver.setSmoothCost(Param::diffuse, SmoothType::one, {0.5, 1.0, 1.0});
		solver.setBound(Param::diffuse, 0.0, 1.0);

		solver.setSmoothCost(Param::roughness, SmoothType::one, 0.05);
		solver.setBound(Param::diffuse, 0.0);

		solver.setSmoothCost(Param::height, SmoothType::zero, 0.01);
		solver.setSmoothCost(Param::height, SmoothType::one, 0.1);

		solver.setConstantSpecular(true);

		solver.setOutputDirectory("Data/example/Output/2/");
		solver.run(31);

		////std::string root = "C:/Users/phgphg/Desktop/CeresSolver/";
		//solver.setInputDirectory("Data/scene7/");
		//solver.setRecordViewIndices({ 1, 2, 3 });
	
		////solver.setDomain(478, 367, 3, 3);
		////solver.setDomain(447, 491, 3, 3);
		////solver.setDomain(400, 450, 100, 100);
		//solver.setDomain(260, 250, 250, 500);
		////solver.setDomain(300, 320, 150, 180);
		////solver.setDomain(200, 200, 600, 600);
	
		//solver.setViewWeightMin(0.01);
		////solver.setViewWeightBias(4.0);
		//solver.setZeroRadius(2);
	
		//solver.setDisabledCameras({ 1,4, 3,  6 });
		////solver.writeVisibilityAtNextRun();

		//solver.setNormalMode(NormalOptMode::heightmap2018);
		//solver.setDifferenceMode(DifferenceMode::forward2);

		//solver.setParamSpace(
		//	ParamSpace::param_diffuse
		//	| ParamSpace::param_specular
		//	| ParamSpace::param_roughness
		//	| ParamSpace::param_normal
		//);
		//solver.setBound(Param::diffuse, 0.0, 1.0);
		//solver.setBound(Param::specular, 0.0, 1.0);
		//solver.setBound(Param::roughness, 0.1, 0.8);
		////solver.setConstantSpecular(true);
		////solver.setConstantRoughness(true);
		////solver.setSmoothCostBase(Param::roughness, 0.235);

		////solver.setChannelWeight({1,2,1});
		//
		//solver.setSmoothCost(Param::diffuse, SmoothType::one,  {0.5, 0.5, 3.0}, 1.0);
		////solver.setSmoothCost(Param::diffuse, SmoothType::one, 0.5, 1.0);
		////solver.setSmoothCost(Param::diffuse, SmoothType::four, {1.0, 1.0, 5.0}, 1.0);
	
		////solver.setSmoothCost(Param::specular, SmoothType::one, 0.2, 2);
		//solver.setSmoothCost(Param::specular, SmoothType::zero, 0.2, 1.0);
		//
		//solver.setSmoothCost(Param::roughness, SmoothType::one, 0.02, 1.0);

	
		////solver.setSmoothCost(Param::height, SmoothType::four, 0.5, 1.0);
		//solver.setSmoothCost(Param::height, SmoothType::one, 0.5, 1.0);
		////solver.setSmoothCost(Param::height, SmoothType::two, 0.1, 1.0);
		////solver.setSmoothCost(Param::height, SmoothType::zero, 0.025, 1.0);
		
		/*solver.invalidateSolution();
		solver.setSmoothCost(Param::height, SmoothType::five, 1.0, 1.0);
		solver.setOutputDirectory("Data/scene7/Output/2.5.5/");
		solver.run(31);

		solver.invalidateSolution();
		solver.setSmoothCost(Param::height, SmoothType::five, 1.0, 2.0);
		solver.setOutputDirectory("Data/scene7/Output/2.5.6/");
		solver.run(31);

		solver.invalidateSolution();
		solver.setSmoothCost(Param::height, SmoothType::zero, 1.0, 1.0);
		solver.setOutputDirectory("Data/scene7/Output/2.5.7/");
		solver.run(31);

		solver.invalidateSolution();
		solver.setSmoothCost(Param::height, SmoothType::zero, 1.0, 2.0);
		solver.setOutputDirectory("Data/scene7/Output/2.5.8/");
		solver.run(31);*/


		/*solver.invalidateSolution();
		solver.setChannelWeight({10,1,1});
		solver.setOutputDirectory("Data/scene7/Output/3.1/");
		solver.run(21);*/


		/*solver.invalidateSolution();
		solver.setChannelWeight({1,1,10});
		solver.setOutputDirectory("Data/scene7/Output/4.2/");
		solver.run(21);*/





		/*solver.setChannelWeight({1.0, 1.0, 10.0});
	
		solver.setDisabledCameras({ 1,3, 4,5,6, 7,9 });
		solver.setOutputDirectory("./Data/scene6/Output/2.8.1/");
		solver.run(31);

		solver.invalidateSolution();
		solver.setDisabledCameras({ 3, 4,5,6, 7,8,9 });
		solver.setOutputDirectory("./Data/scene6/Output/2.8.2/");
		solver.run(31);

		solver.invalidateSolution();
		solver.setDisabledCameras({ 3, 4,5,6, 9 });
		solver.setOutputDirectory("./Data/scene6/Output/2.8.3/");
		solver.run(31);

		solver.invalidateSolution();
		solver.setDisabledCameras({  4,5,6, 7,8,9 });
		solver.setOutputDirectory("./Data/scene6/Output/2.8.4/");
		solver.run(31);

		solver.invalidateSolution();
		solver.setDisabledCameras({ 4,5,6 });
		solver.setOutputDirectory("./Data/scene6/Output/2.8.5/");
		solver.run(31);*/

		/*solver.setDisabledCameras({ 1,3, 4,6, 7,9 });
		solver.setOutputDirectory("./Data/scene6/Output/2.9.11/");
		solver.run(31);*/


	}
	catch (std::runtime_error& e)
	{
		std::cout<<e.what()<<std::endl;
	}
	catch (...)
	{

	}

	int x;
	std::cin >> x;

	return 0;
}
