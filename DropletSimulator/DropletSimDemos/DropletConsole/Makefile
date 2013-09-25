# 
# DropletConsole build system
#

# normal declarations
SHELL=/bin/bash
CXX=g++
AR=ar

.SUFFIXES:
.SUFFIXES: .c .cpp .o

# Path of DropletConsole directory in relation to the Makefile
CONSOLE_DIR=.

# Path where "make release" will copy everything into
OUT_DIR=$(CONSOLE_DIR)/bin/

# Define paths to important things
BULLET_DIR=$(CONSOLE_DIR)/../External/bullet-2.81-rev2613/src
PROGRAM_DIR=$(CONSOLE_DIR)/../../DropletPrograms
EASYBMP_DIR=$(CONSOLE_DIR)/../External/EasyBMP_1.06
SIMLIB_DIR=$(CONSOLE_DIR)/../../DropletSimLibrary
PROJ_DIR=$(CONSOLE_DIR)/../../../ProjectorImages

# Paths to place compiled binaries
BUILD_DIR=$(CONSOLE_DIR)/build
PROGRAM_BUILD_DIR=$(BUILD_DIR)/programs
BULLET_BUILD_DIR=$(BUILD_DIR)/bullet
SIMLIB_BUILD_DIR=$(BUILD_DIR)/sim
EASYBMP_BUILD_DIR=$(BUILD_DIR)/EasyBMP

# Include definitions
PROGRAM_INCLUDE= -I$(PROGRAM_DIR)
BULLET_INCLUDE= -I$(BULLET_DIR)
EASYBMP_INCLUDE= -I$(EASYBMP_DIR)
SIMLIB_INCLUDE= -I$(SIMLIB_DIR)/include

INCLUDE= -I$(CONSOLEDIR)/include $(SIMLIB_INCLUDE) $(PROGRAM_INCLUDE) $(BULLET_INCLUDE) $(EASYBMP_INCLUDE)

# Preprocessor definitions
DEFINE = -DD_SIM_LIB

# Compiler flags - build 32-bit
CFLAGS = -m32 -O3

# Archive flags
ARFLAGS = rv

#
# Build Rules
#


# declare the default make rule to be 'release'
default: release

# declare 'all' as building source files, but not copying everything into a directory
all: DropletConsole

#
# Release - builds DropletConsole and copies the binary and projector images into defined output directory
#

release: DropletConsole | $(OUT_DIR)
	cp $(BUILD_DIR)/$^ $(OUT_DIR)
	cp $(PROJ_DIR)/*.bmp $(OUT_DIR)

#
# DropletConsole - Builds DropletConsole executable
#

# Define compiler flags for DropletConsole
FLAGS= $(CFLAGS) $(DEFINE) $(INCLUDE)

# Define which files need to be built for DropletConsole itself
CONSOLE= $(BUILD_DIR)/main.o

# Rule for how to build those files
$(BUILD_DIR)/%.o: $(CONSOLE_DIR)/src/%.cpp | $(BUILD_DIR) 
	$(CXX) $(FLAGS) -c $< -o $@	

# Rule for how to build and link DropletConsole executable
DropletConsole: $(CONSOLE) | bullet EasyBMP DropletSimLibrary DropletPrograms
	$(CXX) $(FLAGS)  $^ -static -L$(BUILD_DIR) -lDropletPrograms -lDropletSimLibrary -lEasyBMP -lBullet -o $(BUILD_DIR)/$@	

#
# DropletPrograms - Builds all droplet programs into a library
#

# Define compiler flags
D_FLAGS= $(CFLAGS) $(DEFINE) $(SIMLIB_INCLUDE) $(PROGRAM_INCLUDE) $(BULLET_INCLUDE)

# Define which binary files need to be built
PROGRAMS = 
PROGRAMS += $(PROGRAM_BUILD_DIR)/DropletCommTest.o 
PROGRAMS += $(PROGRAM_BUILD_DIR)/DropletRainbow.o
PROGRAMS += $(PROGRAM_BUILD_DIR)/DropletRGBSense.o
PROGRAMS += $(PROGRAM_BUILD_DIR)/DropletStickPullers.o
PROGRAMS += $(PROGRAM_BUILD_DIR)/DropletMarch.o
PROGRAMS += $(PROGRAM_BUILD_DIR)/DropletTurnTest.o
PROGRAMS += $(PROGRAM_BUILD_DIR)/DropletRandomWalk.o
PROGRAMS += $(PROGRAM_BUILD_DIR)/DropletCustomOne.o 
PROGRAMS += $(PROGRAM_BUILD_DIR)/DropletCustomTwo.o
PROGRAMS += $(PROGRAM_BUILD_DIR)/DropletCustomThree.o
PROGRAMS += $(PROGRAM_BUILD_DIR)/DropletCustomFour.o
PROGRAMS += $(PROGRAM_BUILD_DIR)/DropletCustomFive.o
PROGRAMS += $(PROGRAM_BUILD_DIR)/DropletCustomSix.o
PROGRAMS += $(PROGRAM_BUILD_DIR)/DropletCustomSeven.o
PROGRAMS += $(PROGRAM_BUILD_DIR)/DropletCustomEight.o
PROGRAMS += $(PROGRAM_BUILD_DIR)/DropletCustomNine.o
PROGRAMS += $(PROGRAM_BUILD_DIR)/DropletCustomTen.o


# Define how to build them - note because of different directories pattern matching cannot be used as before
$(PROGRAM_BUILD_DIR)/DropletCommTest.o: $(PROGRAM_DIR)/DefaultPrograms/DropletCommTest/DropletCommTest.cpp | $(PROGRAM_BUILD_DIR)
	$(CXX) $(D_FLAGS) -c $< -o $@

$(PROGRAM_BUILD_DIR)/DropletRainbow.o: $(PROGRAM_DIR)/DefaultPrograms/DropletRainbow/DropletRainbow.cpp | $(PROGRAM_BUILD_DIR)
	$(CXX) $(D_FLAGS) -c $< -o $@

$(PROGRAM_BUILD_DIR)/DropletRGBSense.o: $(PROGRAM_DIR)/DefaultPrograms/DropletRGBSense/DropletRGBSense.cpp | $(PROGRAM_BUILD_DIR)
	$(CXX) $(D_FLAGS) -c $< -o $@

$(PROGRAM_BUILD_DIR)/DropletStickPullers.o: $(PROGRAM_DIR)/DefaultPrograms/DropletStickPullers/DropletStickPullers.cpp | $(PROGRAM_BUILD_DIR)
	$(CXX) $(D_FLAGS) -c $< -o $@

$(PROGRAM_BUILD_DIR)/DropletMarch.o: $(PROGRAM_DIR)/DefaultPrograms/DropletMarch/DropletMarch.cpp | $(PROGRAM_BUILD_DIR)
	$(CXX) $(D_FLAGS) -c $< -o $@

$(PROGRAM_BUILD_DIR)/DropletTurnTest.o: $(PROGRAM_DIR)/DefaultPrograms/DropletTurnTest/DropletTurnTest.cpp | $(PROGRAM_BUILD_DIR)
	$(CXX) $(D_FLAGS) -c $< -o $@

$(PROGRAM_BUILD_DIR)/DropletRandomWalk.o: $(PROGRAM_DIR)/DefaultPrograms/DropletRandomWalk/DropletRandomWalk.cpp | $(PROGRAM_BUILD_DIR)
	$(CXX) $(D_FLAGS) -c $< -o $@

$(PROGRAM_BUILD_DIR)/DropletCustomOne.o: $(PROGRAM_DIR)/CustomPrograms/DropletCustomOne/DropletCustomOne.cpp | $(PROGRAM_BUILD_DIR)
	$(CXX) $(D_FLAGS) -c $< -o $@

$(PROGRAM_BUILD_DIR)/DropletCustomTwo.o: $(PROGRAM_DIR)/CustomPrograms/DropletCustomTwo/DropletCustomTwo.cpp | $(PROGRAM_BUILD_DIR)
	$(CXX) $(D_FLAGS) -c $< -o $@

$(PROGRAM_BUILD_DIR)/DropletCustomThree.o: $(PROGRAM_DIR)/CustomPrograms/DropletCustomThree/DropletCustomThree.cpp | $(PROGRAM_BUILD_DIR)
	$(CXX) $(D_FLAGS) -c $< -o $@

$(PROGRAM_BUILD_DIR)/DropletCustomFour.o: $(PROGRAM_DIR)/CustomPrograms/DropletCustomFour/DropletCustomFour.cpp | $(PROGRAM_BUILD_DIR)
	$(CXX) $(D_FLAGS) -c $< -o $@

$(PROGRAM_BUILD_DIR)/DropletCustomFive.o: $(PROGRAM_DIR)/CustomPrograms/DropletCustomFive/DropletCustomFive.cpp | $(PROGRAM_BUILD_DIR)
	$(CXX) $(D_FLAGS) -c $< -o $@

$(PROGRAM_BUILD_DIR)/DropletCustomSix.o: $(PROGRAM_DIR)/CustomPrograms/DropletCustomSix/DropletCustomSix.cpp | $(PROGRAM_BUILD_DIR)
	$(CXX) $(D_FLAGS) -c $< -o $@

$(PROGRAM_BUILD_DIR)/DropletCustomSeven.o: $(PROGRAM_DIR)/CustomPrograms/DropletCustomSeven/DropletCustomSeven.cpp | $(PROGRAM_BUILD_DIR)
	$(CXX) $(D_FLAGS) -c $< -o $@

$(PROGRAM_BUILD_DIR)/DropletCustomEight.o: $(PROGRAM_DIR)/CustomPrograms/DropletCustomEight/DropletCustomEight.cpp | $(PROGRAM_BUILD_DIR)
	$(CXX) $(D_FLAGS) -c $< -o $@

$(PROGRAM_BUILD_DIR)/DropletCustomNine.o: $(PROGRAM_DIR)/CustomPrograms/DropletCustomNine/DropletCustomNine.cpp | $(PROGRAM_BUILD_DIR)
	$(CXX) $(D_FLAGS) -c $< -o $@

$(PROGRAM_BUILD_DIR)/DropletCustomTen.o: $(PROGRAM_DIR)/CustomPrograms/DropletCustomTen/DropletCustomTen.cpp | $(PROGRAM_BUILD_DIR)
	$(CXX) $(D_FLAGS) -c $< -o $@

	
# Define how to build the library
$(BUILD_DIR)/libDropletPrograms.a: $(PROGRAMS) | $(BUILD_DIR)
	$(AR) $(ARFLAGS) $@ -o $^

# Define a shorthand alias for building the library 'DropletPrograms'
DropletPrograms: $(BUILD_DIR)/libDropletPrograms.a

#
# DropletSimLibrary
#

# Define compiler flags
L_FLAGS= $(CFLAGS) $(DEFINE) $(SIMLIB_INCLUDE) $(BULLET_INCLUDE) $(EASYBMP_INCLUDE)

# Define which binaries need to be built
SIMLIB = 
SIMLIB+= $(SIMLIB_BUILD_DIR)/DropletSim.o
SIMLIB+= $(SIMLIB_BUILD_DIR)/DropletTimeControl.o
SIMLIB+= $(SIMLIB_BUILD_DIR)/IDroplet.o
SIMLIB+= $(SIMLIB_BUILD_DIR)/IDropletSimInterface.o
SIMLIB+= $(SIMLIB_BUILD_DIR)/DropletSimInfo.o
SIMLIB+= $(SIMLIB_BUILD_DIR)/DropletUtil.o
SIMLIB+= $(SIMLIB_BUILD_DIR)/IDropletProjector.o
SIMLIB+= $(SIMLIB_BUILD_DIR)/DSimPhysicalObject.o

# Define how the binaries are built
$(SIMLIB_BUILD_DIR)%.o: $(SIMLIB_DIR)/src/%.cpp | $(SIMLIB_BUILD_DIR)
	$(CXX) $(L_FLAGS) -c $< -o $@

# Define how to build libDropletSimLibrary from those binaries
$(BUILD_DIR)/libDropletSimLibrary.a: $(SIMLIB) | $(SIMLIB_BUILD_DIR)
	$(AR) $(ARFLAGS) $@ -o $^

# Define alias 'DropletSimLibrary' that builds the library
DropletSimLibrary: $(BUILD_DIR)/libDropletSimLibrary.a

#
# EasyBMP
#

# Define compiler flags for EasyBMP
EFLAGS= $(CFLAGS) $(DEFINE) $(EASYBMP_INCLUDE)

# Define which file gets built
EASYBMP = $(EASYBMP_BUILD_DIR)/EasyBMP.o

# Define how EasyBMP files get built
$(EASYBMP_BUILD_DIR)/%.o: $(EASYBMP_DIR)/%.cpp | $(EASYBMP_BUILD_DIR)
	$(CXX) $(CFLAGS) -c $< -o $@

# Define how to built a library from the file
$(BUILD_DIR)/libEasyBMP.a: $(EASYBMP) | $(BUILD_DIR)
	$(AR) $(ARFLAGS) $@ -o $^

# Define alias 'EasyBMP' that builds library
EasyBMP: $(BUILD_DIR)/libEasyBMP.a 


#
# Bullet
#

# Define the command line arguments for bullet
B_FLAGS= $(CFLAGS) $(DEPENDS) $(BULLET_INCLUDE)

# Definitions for Bullet LinearMath library
LINMATH = 
LINMATH+=$(BULLET_BUILD_DIR)/btAlignedAllocator.o
LINMATH+=$(BULLET_BUILD_DIR)/btConvexHull.o
LINMATH+=$(BULLET_BUILD_DIR)/btConvexHullComputer.o
LINMATH+=$(BULLET_BUILD_DIR)/btGeometryUtil.o
LINMATH+=$(BULLET_BUILD_DIR)/btPolarDecomposition.o
LINMATH+=$(BULLET_BUILD_DIR)/btQuickprof.o
LINMATH+=$(BULLET_BUILD_DIR)/btSerializer.o 
LINMATH+=$(BULLET_BUILD_DIR)/btVector3.o

$(BULLET_BUILD_DIR)/%.o: $(BULLET_DIR)/LinearMath/%.cpp |$(BULLET_BUILD_DIR)
	$(CXX) $(B_FLAGS) -c $< -o $@

# Definition for Bullet BroadCollision module (Part of BulletCollision)
BCOLLISION = 
BCOLLISION += $(BULLET_BUILD_DIR)/btAxisSweep3.o 
BCOLLISION += $(BULLET_BUILD_DIR)/btBroadphaseProxy.o 
BCOLLISION += $(BULLET_BUILD_DIR)/btCollisionAlgorithm.o
BCOLLISION += $(BULLET_BUILD_DIR)/btDbvtBroadphase.o
BCOLLISION += $(BULLET_BUILD_DIR)/btDbvt.o 
BCOLLISION += $(BULLET_BUILD_DIR)/btDispatcher.o 
BCOLLISION += $(BULLET_BUILD_DIR)/btMultiSapBroadphase.o	     
BCOLLISION += $(BULLET_BUILD_DIR)/btOverlappingPairCache.o 
BCOLLISION += $(BULLET_BUILD_DIR)/btQuantizedBvh.o 
BCOLLISION += $(BULLET_BUILD_DIR)/btSimpleBroadphase.o

$(BULLET_BUILD_DIR)/%.o: $(BULLET_DIR)/BulletCollision/BroadphaseCollision/%.cpp |$(BULLET_BUILD_DIR)
	$(CXX) $(B_FLAGS) -c $< -o $@

# Definitions for BulletDispatch module (Part of BulletCollision)
DISPATCH= 
DISPATCH+= $(BULLET_BUILD_DIR)/btActivatingCollisionAlgorithm.o
DISPATCH+= $(BULLET_BUILD_DIR)/btBox2dBox2dCollisionAlgorithm.o
DISPATCH+= $(BULLET_BUILD_DIR)/btBoxBoxCollisionAlgorithm.o
DISPATCH+= $(BULLET_BUILD_DIR)/btBoxBoxDetector.o
DISPATCH+= $(BULLET_BUILD_DIR)/btCollisionDispatcher.o
DISPATCH+= $(BULLET_BUILD_DIR)/btCollisionObject.o
DISPATCH+= $(BULLET_BUILD_DIR)/btCollisionWorld.o
DISPATCH+= $(BULLET_BUILD_DIR)/btCompoundCollisionAlgorithm.o
DISPATCH+= $(BULLET_BUILD_DIR)/btConvex2dConvex2dAlgorithm.o
DISPATCH+= $(BULLET_BUILD_DIR)/btConvexConcaveCollisionAlgorithm.o
DISPATCH+= $(BULLET_BUILD_DIR)/btConvexConvexAlgorithm.o
DISPATCH+= $(BULLET_BUILD_DIR)/btConvexPlaneCollisionAlgorithm.o
DISPATCH+= $(BULLET_BUILD_DIR)/btDefaultCollisionConfiguration.o
DISPATCH+= $(BULLET_BUILD_DIR)/btEmptyCollisionAlgorithm.o
DISPATCH+= $(BULLET_BUILD_DIR)/btGhostObject.o
DISPATCH+= $(BULLET_BUILD_DIR)/btInternalEdgeUtility.o
DISPATCH+= $(BULLET_BUILD_DIR)/btManifoldResult.o
DISPATCH+= $(BULLET_BUILD_DIR)/btSimulationIslandManager.o
DISPATCH+= $(BULLET_BUILD_DIR)/btSphereBoxCollisionAlgorithm.o
DISPATCH+= $(BULLET_BUILD_DIR)/btSphereSphereCollisionAlgorithm.o
DISPATCH+= $(BULLET_BUILD_DIR)/btSphereTriangleCollisionAlgorithm.o
DISPATCH+= $(BULLET_BUILD_DIR)/btUnionFind.o
DISPATCH+= $(BULLET_BUILD_DIR)/SphereTriangleDetector.o

$(BULLET_BUILD_DIR)/%.o: $(BULLET_DIR)/BulletCollision/CollisionDispatch/%.cpp |$(BULLET_BUILD_DIR)
	$(CXX) $(B_FLAGS) -c $< -o $@

# Definitions for GImpact module (Part of BulletCollision)
IMPACT =
IMPACT+= $(BULLET_BUILD_DIR)/btContactProcessing.o
IMPACT+= $(BULLET_BUILD_DIR)/btGenericPoolAllocator.o
IMPACT+= $(BULLET_BUILD_DIR)/btGImpactBvh.o
IMPACT+= $(BULLET_BUILD_DIR)/btGImpactCollisionAlgorithm.o
IMPACT+= $(BULLET_BUILD_DIR)/btGImpactQuantizedBvh.o
IMPACT+= $(BULLET_BUILD_DIR)/btGImpactShape.o
IMPACT+= $(BULLET_BUILD_DIR)/btTriangleShapeEx.o
IMPACT+= $(BULLET_BUILD_DIR)/gim_box_set.o
IMPACT+= $(BULLET_BUILD_DIR)/gim_contact.o
IMPACT+= $(BULLET_BUILD_DIR)/gim_memory.o
IMPACT+= $(BULLET_BUILD_DIR)/gim_tri_collision.o

$(BULLET_BUILD_DIR)/%.o: $(BULLET_DIR)/BulletCollision/Gimpact/%.cpp |$(BULLET_BUILD_DIR)
	$(CXX) $(B_FLAGS) -c $< -o $@

# Definitions for CollisionShapes module (Part of BulletCollision)
SHAPES =
SHAPES+= $(BULLET_BUILD_DIR)/btBox2dShape.o
SHAPES+= $(BULLET_BUILD_DIR)/btBoxShape.o
SHAPES+= $(BULLET_BUILD_DIR)/btBvhTriangleMeshShape.o
SHAPES+= $(BULLET_BUILD_DIR)/btCapsuleShape.o
SHAPES+= $(BULLET_BUILD_DIR)/btCollisionShape.o
SHAPES+= $(BULLET_BUILD_DIR)/btCompoundShape.o
SHAPES+= $(BULLET_BUILD_DIR)/btConcaveShape.o
SHAPES+= $(BULLET_BUILD_DIR)/btConeShape.o
SHAPES+= $(BULLET_BUILD_DIR)/btConvex2dShape.o
SHAPES+= $(BULLET_BUILD_DIR)/btConvexHullShape.o
SHAPES+= $(BULLET_BUILD_DIR)/btConvexInternalShape.o
SHAPES+= $(BULLET_BUILD_DIR)/btConvexPointCloudShape.o
SHAPES+= $(BULLET_BUILD_DIR)/btConvexPolyhedron.o
SHAPES+= $(BULLET_BUILD_DIR)/btConvexShape.o
SHAPES+= $(BULLET_BUILD_DIR)/btConvexTriangleMeshShape.o
SHAPES+= $(BULLET_BUILD_DIR)/btCylinderShape.o
SHAPES+= $(BULLET_BUILD_DIR)/btEmptyShape.o
SHAPES+= $(BULLET_BUILD_DIR)/btHeightfieldTerrainShape.o
SHAPES+= $(BULLET_BUILD_DIR)/btMinkowskiSumShape.o
SHAPES+= $(BULLET_BUILD_DIR)/btMultimaterialTriangleMeshShape.o
SHAPES+= $(BULLET_BUILD_DIR)/btMultiSphereShape.o
SHAPES+= $(BULLET_BUILD_DIR)/btOptimizedBvh.o
SHAPES+= $(BULLET_BUILD_DIR)/btPolyhedralConvexShape.o
SHAPES+= $(BULLET_BUILD_DIR)/btScaledBvhTriangleMeshShape.o
SHAPES+= $(BULLET_BUILD_DIR)/btShapeHull.o
SHAPES+= $(BULLET_BUILD_DIR)/btSphereShape.o
SHAPES+= $(BULLET_BUILD_DIR)/btStaticPlaneShape.o
SHAPES+= $(BULLET_BUILD_DIR)/btStridingMeshInterface.o
SHAPES+= $(BULLET_BUILD_DIR)/btTetrahedronShape.o
SHAPES+= $(BULLET_BUILD_DIR)/btTriangleBuffer.o
SHAPES+= $(BULLET_BUILD_DIR)/btTriangleCallback.o
SHAPES+= $(BULLET_BUILD_DIR)/btTriangleIndexVertexArray.o
SHAPES+= $(BULLET_BUILD_DIR)/btTriangleIndexVertexMaterialArray.o
SHAPES+= $(BULLET_BUILD_DIR)/btTriangleMesh.o
SHAPES+= $(BULLET_BUILD_DIR)/btTriangleMeshShape.o
SHAPES+= $(BULLET_BUILD_DIR)/btUniformScalingShape.o

$(BULLET_BUILD_DIR)/%.o: $(BULLET_DIR)/BulletCollision/CollisionShapes/%.cpp |$(BULLET_BUILD_DIR)
	$(CXX) $(B_FLAGS) -c $< -o $@

# Definitions for Bullet NarrowPhaseCollision module (Part of BulletCollision)
NCOLLISION=
NCOLLISION+= $(BULLET_BUILD_DIR)/btContinuousConvexCollision.o
NCOLLISION+= $(BULLET_BUILD_DIR)/btConvexCast.o
NCOLLISION+= $(BULLET_BUILD_DIR)/btGjkConvexCast.o
NCOLLISION+= $(BULLET_BUILD_DIR)/btGjkEpa2.o
NCOLLISION+= $(BULLET_BUILD_DIR)/btGjkEpaPenetrationDepthSolver.o
NCOLLISION+= $(BULLET_BUILD_DIR)/btGjkPairDetector.o
NCOLLISION+= $(BULLET_BUILD_DIR)/btMinkowskiPenetrationDepthSolver.o
NCOLLISION+= $(BULLET_BUILD_DIR)/btPersistentManifold.o
NCOLLISION+= $(BULLET_BUILD_DIR)/btPolyhedralContactClipping.o
NCOLLISION+= $(BULLET_BUILD_DIR)/btRaycastCallback.o
NCOLLISION+= $(BULLET_BUILD_DIR)/btSubSimplexConvexCast.o
NCOLLISION+= $(BULLET_BUILD_DIR)/btVoronoiSimplexSolver.o

$(BULLET_BUILD_DIR)/%.o: $(BULLET_DIR)/BulletCollision/NarrowPhaseCollision/%.cpp |$(BULLET_BUILD_DIR)
	$(CXX) $(B_FLAGS) -c $< -o $@

# Combine the above definitions into a list of files to build for the BulletCollision module
COLLISION = $(BCOLLISION) $(DISPATCH) $(IMPACT) $(NCOLLISION) $(SHAPES)


# Definitions for BulletDynamics module
DYNAMICS =
DYNAMICS+= $(BULLET_BUILD_DIR)/btKinematicCharacterController.o
DYNAMICS+= $(BULLET_BUILD_DIR)/btConeTwistConstraint.o
DYNAMICS+= $(BULLET_BUILD_DIR)/btContactConstraint.o
DYNAMICS+= $(BULLET_BUILD_DIR)/btGearConstraint.o
DYNAMICS+= $(BULLET_BUILD_DIR)/btGeneric6DofConstraint.o
DYNAMICS+= $(BULLET_BUILD_DIR)/btGeneric6DofSpringConstraint.o
DYNAMICS+= $(BULLET_BUILD_DIR)/btHinge2Constraint.o
DYNAMICS+= $(BULLET_BUILD_DIR)/btHingeConstraint.o
DYNAMICS+= $(BULLET_BUILD_DIR)/btPoint2PointConstraint.o
DYNAMICS+= $(BULLET_BUILD_DIR)/btSequentialImpulseConstraintSolver.o
DYNAMICS+= $(BULLET_BUILD_DIR)/btSliderConstraint.o
DYNAMICS+= $(BULLET_BUILD_DIR)/btSolve2LinearConstraint.o
DYNAMICS+= $(BULLET_BUILD_DIR)/btTypedConstraint.o
DYNAMICS+= $(BULLET_BUILD_DIR)/btUniversalConstraint.o
DYNAMICS+= $(BULLET_BUILD_DIR)/btDiscreteDynamicsWorld.o
DYNAMICS+= $(BULLET_BUILD_DIR)/btRigidBody.o
DYNAMICS+= $(BULLET_BUILD_DIR)/btSimpleDynamicsWorld.o
DYNAMICS+= $(BULLET_BUILD_DIR)/Bullet-C-API.o
DYNAMICS+= $(BULLET_BUILD_DIR)/btRaycastVehicle.o
DYNAMICS+= $(BULLET_BUILD_DIR)/btWheelInfo.o

$(BULLET_BUILD_DIR)/%.o: $(BULLET_DIR)/BulletDynamics/Character/%.cpp |$(BULLET_BUILD_DIR)
	$(CXX) $(B_FLAGS) -c $< -o $@

$(BULLET_BUILD_DIR)/%.o: $(BULLET_DIR)/BulletDynamics/ConstraintSolver/%.cpp |$(BULLET_BUILD_DIR)
	$(CXX) $(B_FLAGS) -c $< -o $@

$(BULLET_BUILD_DIR)/%.o: $(BULLET_DIR)/BulletDynamics/Dynamics/%.cpp |$(BULLET_BUILD_DIR)
	$(CXX) $(B_FLAGS) -c $< -o $@

$(BULLET_BUILD_DIR)/%.o: $(BULLET_DIR)/BulletDynamics/Vehicle/%.cpp |$(BULLET_BUILD_DIR)
	$(CXX) $(B_FLAGS) -c $< -o $@

# Combine above definitions into a list of files to build when building Bullet
BULLET= $(LINMATH) $(COLLISION) $(DYNAMICS)

# Definition of building libBullet
$(BUILD_DIR)/libBullet.a: $(BULLET) | $(BULLET_BUILD_DIR)
	$(AR) $(ARFLAGS) $@ -o $^

# Alias 'bullet' for building libBullet
bullet: $(BUILD_DIR)/libBullet.a


#
# Clean-up
#

# Clean up DropletPrograms
clean-programs:
	rm -rf $(PROGRAMS)
	rm -rf $(PROGRAM_BUILD_DIR)
	rm -rf $(BUILD_DIR)/libDropletPrograms.a

# Clean up Bullet
clean-bullet: 
	rm -rf $(BULLET)
	rm -rf $(BULLET_BUILD_DIR)
	rm -rf $(BUILD_DIR)/libBullet.a

# Clean up EasyBMP
clean-easybmp:
	rm -rf $(EASYBMP)
	rm -rf $(EASYBMP_BUILD_DIR)
	rm -rf $(BUILD_DIR)/libEasyBMP.a

# Clean up DropletSimLibrary
clean-simlibrary:
	rm -rf $(SIMLIB)
	rm -rf $(BUILD_DIR)/libDropletSimLibrary.a

# Clean up DropletConsole
clean-console:
	rm -rf DropletConsole

# Default 'clean' removes all intermediary files but not release directory
clean: clean-bullet clean-easybmp clean-programs clean-simlibrary clean-console
	rm -rf $(BUILD_DIR)

# Clean up release files
clean-release:
	rm -rf $(OUT_DIR)

# Clean all removes everything
clean-all: clean clean-release


# Definitions to create intermediary and output directories

$(OUT_DIR):
	mkdir $(OUT_DIR)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(SIMLIB_BUILD_DIR): | $(BUILD_DIR)
	mkdir $(SIMLIB_BUILD_DIR)

$(BULLET_BUILD_DIR): | $(BUILD_DIR)
	mkdir $(BULLET_BUILD_DIR)

$(EASYBMP_BUILD_DIR): | $(BUILD_DIR)
	mkdir $(EASYBMP_BUILD_DIR)

$(PROGRAM_BUILD_DIR): | $(BUILD_DIR)
	mkdir $(PROGRAM_BUILD_DIR)
