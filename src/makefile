XX=g++
GUROBILIB = -L /home/peterhao/Tools/gurobi900/linux64/lib/ -lgurobi_g++5.2 -lgurobi90
GUROBIINC = -I /home/peterhao/Tools/gurobi900/linux64/include/

CARGS= -m64 -std=c++11 -fpermissive 


objects=Grain128Tests AcornTests KreyviumTests Grain128Bounds Grain128aBounds GrainV1Bounds PlantletBounds
all:$(objects)
#$(objects): %.o:%.cpp
#	$(XX) -c $(CARGS) $(GUROBIINC) $(GUROBILIB) $< -o $@ 
Grain128Tests:MainGrain128Upload.cpp Grain128MilpDivisionClass.h Grain128CubeRunner.h DivisionPropertyBasic.h 
	$(XX) $(CARGS) -o Grain128Tests MainGrain128Upload.cpp $(GUROBIINC) $(GUROBILIB)
AcornTests:MainAcornUpload.cpp AcornMilpDivisionClass.h DivisionPropertyBasic.h 
	$(XX) $(CARGS) -o AcornTests MainAcornUpload.cpp $(GUROBIINC) $(GUROBILIB)
KreyviumTests:MainKreyviumUpload.cpp KreyviumMilpDivisionClass.h DivisionPropertyBasic.h 
	$(XX) $(CARGS) -o KreyviumTests MainKreyviumUpload.cpp $(GUROBIINC) $(GUROBILIB)
Grain128Bounds:MainGrain128Bound.cpp Grain128MilpDivisionClass.h DivisionPropertyBasic.h
	$(XX) $(CARGS) -o Grain128Bounds MainGrain128Bound.cpp $(GUROBIINC) $(GUROBILIB)
Grain128aBounds:MainGrain128aBound.cpp Grain128aMilpDivisionClass.h DivisionPropertyBasic.h
	$(XX) $(CARGS) -o Grain128aBounds MainGrain128aBound.cpp $(GUROBIINC) $(GUROBILIB)
GrainV1Bounds:MainGrainV1Bound.cpp GrainV1MilpDivisionClass.h DivisionPropertyBasic.h
	$(XX) $(CARGS) -o GrainV1Bounds MainGrainV1Bound.cpp $(GUROBIINC) $(GUROBILIB)
PlantletBounds:MainPlantletBound.cpp PlantletMilpDivisionClass.h DivisionPropertyBasic.h
	$(XX) $(CARGS) -o PlantletBounds MainPlantletBound.cpp $(GUROBIINC) $(GUROBILIB)
clean:
	rm $(objects)

