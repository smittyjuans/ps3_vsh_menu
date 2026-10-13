TARGET		:=	sr2_trainer
TITLE		:=	Saints Row 2 PS3 Live Trainer
PROJECT		:=	SR2_Trainer_SPRX

# Point to standard PSL1GHT environment libraries
include $(PSL1GHT)/base_rules

# Compiler parameters
CFILES		:=	main.cpp
LDFLAGS		:=	-lpad -lsysfs

include $(PSL1GHT)/ppu_rules
