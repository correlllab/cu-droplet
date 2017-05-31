#----------------------------------------------------------------------------
# DROPLETS MAKEFILE
# Based heavily on WinAVR Makefile Template written by Eric B. Weddington, et al.
#
# On command line:
#
# make all = Make software.
# make clean = Clean out built project files.
# To rebuild project do "make clean" then "make all".
#----------------------------------------------------------------------------

# Target file name (without extension).
# This should match your AtmelStudio Project Name
TARGET = Droplets

# The base directory of your Atmel Studio installation folder.
# This serves as the base location for accessing key #include files.
ATMEL_STUDIO_PATH = "C:/Program Files (x86)/Atmel/Studio/7.0/"

# List your user C source file(s) here.
USER_FILES = \
../pong.c \
#../user_template.c \



#----------------------------------------------------------------------------
# ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - !
# - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! -
# ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - !
# DO NOT EDIT ANYTHING BELOW THIS SECTION UNLESS YOU'RE SURE YOU SHOULD BE
# ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - !
# - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! -ff
# ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - ! - !
#----------------------------------------------------------------------------

# Optimization level, can be [0, 1, 2, 3, s]. 
#     0 = turn off optimization. s = optimize for size.
#     (Note: 3 is not always the best optimization level. See avr-libc FAQ.)
OPT = 2

# MCU name
MCU = atxmega128a3u
F_CPU = 32000000

# Output format. (can be srec, ihex, binary)
FORMAT = ihex

# Object files directory
#     To put object files in current directory, use a dot (.), do NOT make
#     this an empty or blank macro!
# JOHN NOTE: This directory appears to be relative to where the *.c file containing
# main is (maybe??)
OBJDIR = ../build/src

# List C source files here. (C dependencies are automatically generated.)
SRC = $(USER_FILES) \
../src/droplet_base.c \
../src/droplet_init.c \
../src/eeprom_driver.c \
../src/firefly_sync.c \
../src/flash_api.c \
../src/i2c.c \
../src/ir_comm.c \
../src/ir_led.c \
../src/ir_sensor.c \
../src/localization.c \
../src/matrix_utils.c \
../src/mic.c \
../src/motor.c \
../src/pc_comm.c \
../src/power.c \
../src/random.c \
../src/range_algs.c \
../src/rgb_led.c \
../src/rgb_sensor.c \
../src/scheduler.c \
../src/serial_handler.c \
../src/speaker.c

ASRC =  \
../src/droplet_base_asm.S \
../src/flash_api_asm.S \
../src/sp_driver.S

C_INCLUDE_DIRS = \
-I ../include \
-I ../

SYS_INCLUDE_DIRS = \
-I $(ATMEL_STUDIO_PATH)Packs/atmel/XMEGAA_DFP/1.1.68/include \
-B $(ATMEL_STUDIO_PATH)Packs/atmel/XMEGAA_DFP/1.1.68/gcc/dev/atxmega128a3u

# Debugging format.
#     Native formats for AVR-GCC's -g are dwarf-2 [default] or stabs.
#     AVR Studio 4.10 requires dwarf-2.
#     AVR [Extended] COFF format requires stabs, plus an avr-objcopy run.
DEBUG = dwarf-2

# Compiler flag to set the C Standard level.
#     c89   = "ANSI" C
#     gnu89 = c89 plus GCC extensions
#     c99   = ISO C99 standard (not yet fully implemented)
#     gnu99 = c99 plus GCC extensions
CSTANDARD = -std=gnu99

# Place -D or -U options here for C sources
DEFS = -DF_CPU=$(F_CPU)UL

#---------------- Compiler Options C ----------------
#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
CFLAGS = -g$(DEBUG)
CFLAGS += $(DEFS)
CFLAGS += -O$(OPT)
#CFLAGS += -funsigned-char
#CFLAGS += -funsigned-bitfields
CFLAGS += -ffunction-sections
CFLAGS += -fdata-sections
CFLAGS += -fpack-struct
CFLAGS += -fshort-enums
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Wstrict-prototypes
#CFLAGS += -mshort-calls
#CFLAGS += -fno-unit-at-a-time
#CFLAGS += -Wundef
#CFLAGS += -Wunreachable-code
#CFLAGS += -Wsign-compare
CFLAGS += -Wa,-adhlns=$(<:%.c=$(OBJDIR)/%.lst)
CFLAGS += $(CSTANDARD)

#---------------- Library Options ----------------
# Minimalistic printf version
PRINTF_LIB_MIN = -Wl,-u,vfprintf -lprintf_min

# Floating point printf version (requires MATH_LIB = -lm below)
PRINTF_LIB_FLOAT = -Wl,-u,vfprintf -lprintf_flt

# If this is left blank, then it will use the Standard printf version.
PRINTF_LIB = $(PRINTF_LIB_FLOAT)
#PRINTF_LIB = $(PRINTF_LIB_MIN)
#PRINTF_LIB = $(PRINTF_LIB_FLOAT)


# Minimalistic scanf version
SCANF_LIB_MIN = -Wl,-u,vfscanf -lscanf_min

# Floating point + %[ scanf version (requires MATH_LIB = -lm below)
SCANF_LIB_FLOAT = -Wl,-u,vfscanf -lscanf_flt

# If this is left blank, then it will use the Standard scanf version.
#SCANF_LIB = $(SCANF_LIB_MIN)
SCANF_LIB = $(SCANF_LIB_FLOAT)

MATH_LIB = -lm

BOOTSECTIONSTART = 0x20000

#---------------- Linker Options ----------------
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDFLAGS = -Wl,-Map=$(TARGET).map,--cref
LDFLAGS += $(EXTMEMOPTS)
LDFLAGS += $(PRINTF_LIB) $(SCANF_LIB) $(MATH_LIB)
LDFLAGS += -Wl,--gc-sections -Wl,-section-start=.BOOT=$(BOOTSECTIONSTART)




# Define programs and commands.
SHELL       = cmd
CC          = $(ATMEL_STUDIO_PATH)toolchain/avr8/avr8-gnu-toolchain/bin/avr-gcc.exe
OBJCOPY     = avr-objcopy
OBJDUMP     = avr-objdump
SIZE        = avr-size
NM          = avr-nm
REMOVE      = rm -f
REMOVEDIR   = rm -rf
COPY        = cp


# Define Messages
# English
MSG_ERRORS_NONE = Errors: none
MSG_BEGIN = ++++++++++++++++ begin ++++++++++++++++
MSG_END = ++++++++++++++++  end  ++++++++++++++++
MSG_SIZE_AFTER = Size:
MSG_FLASH = Creating load file for Flash:
MSG_EEPROM = Creating load file for EEPROM:
MSG_EXTENDED_LISTING = Creating Extended Listing:
MSG_SYMBOL_TABLE = Creating Symbol Table:
MSG_LINKING = Linking:
MSG_COMPILING = Compiling C:
MSG_ASSEMBLING = Assembling:
MSG_CLEANING = Cleaning project:

# Display size of file.
ELFSIZE = $(SIZE) --mcu=$(MCU) --format=avr $(TARGET).elf

sizeafter:
	@printf "\t\n"
	@$(ELFSIZE)
	@printf "----------------\n"

# Display compiler version information.
gccversion : 
	@$(CC) --version
	@printf "\t\n"

# Define all object files.
OBJ = $(SRC:%.c=$(OBJDIR)/%.o) $(ASRC:%.S=$(OBJDIR)/%.o)

# Define all listing files.
LST = $(SRC:%.c=$(OBJDIR)/%.lst) $(ASRC:%.s=$(OBJDIR)/%.lst) 

# Compiler flags to generate dependency files.
GENDEPFLAGS = -MMD -MP -MF .dep/$(@F:%.o=%.d)

# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS = -mmcu=$(MCU) $(CFLAGS) $(SYS_INCLUDE_DIRS) $(C_INCLUDE_DIRS) $(GENDEPFLAGS)
ALL_ASFLAGS = -Wa,-adhlns=$(<:%.S=$(OBJDIR)/%.lst),-g$(DEBUG) $(DEFS) -x assembler-with-cpp -mmcu=$(MCU) $(SYS_INCLUDE_DIRS) $(GENDEPFLAGS) -Wa,-g

# Default target.
all: begin gccversion build sizeafter end

# Change the build target to build a HEX file or a library.
build: elf hex eep lss sym
#build: lib


elf: $(TARGET).elf
hex: $(TARGET).hex
eep: $(TARGET).eep
lss: $(TARGET).lss
sym: $(TARGET).sym
LIBNAME=lib$(TARGET).a
lib: $(LIBNAME)



# Eye candy.
# AVR Studio 3.x does not check make's exit code but relies on
# the following magic strings to be generated by the compile job.
begin:
	@printf "\t\n"
	@echo $(MSG_BEGIN)
	@printf "\t\n"

end:
	@printf "\t\n"
	@echo $(MSG_END)
	@printf "\t\n"

# Create final output files (.hex, .eep) from ELF output file.
%.hex: %.elf
	@printf "\t\n"
	@echo $(MSG_FLASH) $@
	$(OBJCOPY) -O $(FORMAT) -R .eeprom -R .fuse -R .lock -R .signature $< $@

%.eep: %.elf
	@printf "\t\n"
	@echo $(MSG_EEPROM) $@
	-$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" \
	--change-section-lma .eeprom=0 --no-change-warnings -O $(FORMAT) $< $@ || exit 0

# Create extended listing file from ELF output file.
%.lss: %.elf
	@printf "\t\n"
	@echo $(MSG_EXTENDED_LISTING) $@
	$(OBJDUMP) -h -S -z $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	@printf "\t\n"
	@echo $(MSG_SYMBOL_TABLE) $@
	$(NM) -n $< > $@

# Link: create ELF output file from object files.
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(OBJ)
%.elf: $(OBJ)
	@printf "\t\n"
	@echo $(MSG_LINKING) $@
	$(CC) $(ALL_CFLAGS) $^ --output $@ $(LDFLAGS)


# Compile: create object files from C source files.
$(OBJDIR)/%.o : %.c
	@printf "\t\n"
	@echo $(MSG_COMPILING) $<
	$(CC) -c $(ALL_CFLAGS) $< -o $@ 

# Compile: create assembler files from C source files.
%.s : %.c
	$(CC) -S $(ALL_CFLAGS) $< -o $@

# Assemble: create object files from assembler source files.
$(OBJDIR)/%.o : %.S
	@printf "\t\n"
	@echo $(MSG_ASSEMBLING) $<
	$(CC) -c $(ALL_ASFLAGS) $< -o $@

# Target: clean project.
clean: begin clean_list end

clean_list :
	@printf "\t\n"
	@echo $(MSG_CLEANING)
	$(REMOVE) $(TARGET).hex
	$(REMOVE) $(TARGET).eep
	$(REMOVE) $(TARGET).cof
	$(REMOVE) $(TARGET).elf
	$(REMOVE) $(TARGET).map
	$(REMOVE) $(TARGET).sym
	$(REMOVE) $(TARGET).lss
	$(REMOVE) $(SRC:%.c=$(OBJDIR)/%.o)
	$(REMOVE) $(SRC:%.c=$(OBJDIR)/%.lst)
	$(REMOVE) $(SRC:%.c=$(OBJDIR)/%.d)
	$(REMOVE) $(ASRC:%.S=$(OBJDIR)/%.o)
	$(REMOVE) $(ASRC:%.S=$(OBJDIR)/%.lst)
	$(REMOVE) $(ASRC:%.S=$(OBJDIR)/%.d)
	$(REMOVE) $(SRC:.c=.s)
	$(REMOVE) $(SRC:.c=.d)
	$(REMOVE) $(SRC:.c=.i)
	$(REMOVE) $(ASRC:.S=.d)
	$(REMOVE) $(ASRC:.S=.i)
	$(REMOVEDIR) .dep
    

# Create object files directory
$(shell if not exist "$(OBJDIR)" mkdir "$(OBJDIR)")


# Include the dependency files.
-include $(shell if not exist ".dep" mkdir ".dep") $(wildcard .dep/*)