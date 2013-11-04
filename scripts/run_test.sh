#!/bin/sh

ITER_FIRST=500
ITER_LAST=1300
ITER_STEP=50

#ovnis settings
SUMOCONFIG=scenario_capacity.sumocfg
START_TIME=0
STOP_TIME=1800
ROOT_FOLDER=/Users/agatagrzybek/workspace/ovnis/scenarios/Highway/routes_capacity_bypass/

# fce settings
ROUTING_STRATEGIES="noRouting"
ROUTING_STRATEGIES_PROB="1"
CHEATERS_RATIO=0;
# ACCIDENT_START=0;
# ACCIDENT_STEP=10;

APP_PATH="/Users/agatagrzybek/workspace/ovnis/test/ovnisSample_static"
SUMO_PATH="/opt/sumo/bin/sumo"
ENV="mac"
CMD="capacity" # or cheaters

echo "Number of arguments: "$#
if [ $1 ]; then ITER_FIRST=$1; fi
echo "ITER_FIRST "$ITER_FIRST

if [ $2 ]; then ITER_LAST=$2; fi
echo "ITER_LAST "$ITER_LAST

if [ $3 ]; then ITER_STEP=$3; fi
echo "ITER_STEP "$ITER_STEP

if [ $4 ]; then START_TIME=$4; fi
echo "START_TIME "$START_TIME

if [ $5 ]; then STOP_TIME=$5; fi
echo "ITER_STEP "$STOP_TIME

if [ $6 ]; then ROUTING_STRATEGIES=$6; fi
echo "ROUTING_STRATEGIES "$ROUTING_STRATEGIES

if [ $7 ]; then START_TIME=$7; fi
echo "ROUTING_STRATEGIES_PROB "$ROUTING_STRATEGIES_PROB

if [ $8 ]; then CHEATERS_RATIO=$8; fi
echo "CHEATERS_RATIO "$CHEATERS_RATIO

if [ $9 ]; then ROOT_FOLDER=$9; fi
echo "ROOT_FOLDER "$ROOT_FOLDER

if [ $10 ]; then SUMOCONFIG="${10}"; fi
echo "SUMOCONFIG "$SUMOCONFIG

if [ $11 ]; then ENV="${11}"; fi
echo "ENV "$ENV
if [ $ENV = "cluster" ]; then
	echo "export LD_LIBRARY_PATH=/home/users/agrzybek/ovnis/repos/ns-allinone-3.18/ns-3.18/build/"
	export LD_LIBRARY_PATH=/home/users/agrzybek/ovnis/repos/ns-allinone-3.18/ns-3.18/build/
	APP_PATH="/home/users/agrzybek/ovnis/repos/ovnis-master/test/ovnisSample_static"
	SUMO_PATH="/home/users/agrzybek/bin/sumo"
	echo "APP_PATH=/home/users/agrzybek/ovnis/repos/ovnis-master/test/ovnisSample_static"
	echo "SUMO_PATH=/home/users/agrzybek/bin/sumo"
fi

if [ $12 ]; then CMD="${12}"; fi
echo "CMD "$CMD


NUM_ITER=$(((ITER_LAST - $ITER_FIRST) / ITER_STEP))
echo "Num iter "$NUM_ITER

for ITER in $(seq 0 $NUM_ITER); do
	FOLDER=$((ITER_FIRST + ITER * ITER_STEP ))
	if [ $CMD = "capacity" ]; then
		OUTPUT_FOLDER=$ROOT_FOLDER$FOLDER"/"
	fi
	if [ $CMD = "cheaters" ]; then
		OUTPUT_FOLDER=$ROOT_FOLDER$CHEATERS_RATIO
		mkdir $OUTPUT_FOLDER
		OUTPUT_FOLDER=$ROOT_FOLDER$CHEATERS_RATIO/$FOLDER"/"
		mkdir $OUTPUT_FOLDER
		copy="cp ${ROOT_FOLDER}../scenario.sumocfg ${OUTPUT_FOLDER}"
		echo $copy
		$copy
	fi

	echo "Iter ${ITER} folder ${FOLDER}"
	echo "SUMOCONFIG ${OUTPUT_FOLDER}${SUMOCONFIG}"
	PROGRAM="${APP_PATH} \
	--sumoConfig=${SUMOCONFIG} \
	--sumoPath=${SUMO_PATH} \
	--scenarioFolder=${OUTPUT_FOLDER} \
	--startTime=${START_TIME} \
	--stopTime=${STOP_TIME} \
	--outputFolder=${OUTPUT_FOLDER} \
	--routingStrategies=${ROUTING_STRATEGIES} \
	--routingStrategiesProbabilities=${ROUTING_STRATEGIES_PROB} \
	--cheatersRatio=${CHEATERS_RATIO}"
	echo $PROGRAM
	$PROGRAM
done



