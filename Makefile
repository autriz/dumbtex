CC			:= g++
TARGET		:= "dumbtex"
BUILDDIR	:= build
SRCDIR		:= src
CFLAGS		:= -std=c++20 -g
CPRODFLAGS 	:= -std=c++20 -g -O2
SRCEXT		:= cpp
SOURCES 	:= $(wildcard $(SRCDIR)/*.$(SRCEXT))
OBJECTS		:= $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(SOURCES:.$(SRCEXT)=.o))

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@printf "\e[33m\e[1mBuilding...\e[0m\n";
	@mkdir -p $(BUILDDIR)
	@echo "  $(notdir $@) from $(notdir $<)"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJECTS)
	@printf "\e[35m\e[1mLinking...\e[0m\n";
	@echo "  $(notdir $(OBJECTS))"
	@$(CC) $(CFLAGS) -o $@ $^ main.cpp


PHONY: clean prod
clean:
	@printf "\e[31m\e[1mCleaning...\e[0m\n"
	@echo "  /$(BUILDDIR)"
	@echo "  /$(TARGET)"
	@$(RM) -r $(BUILDDIR) $(OBJECTS)
	@$(RM) "./$(TARGET)"

prod:
	@mkdir -p $(BUILDDIR)
	@for source in $(basename $(notdir $(SOURCES))); do\
		printf "\e[33m\e[1mBuilding...\e[0m\n";\
		echo "  $$source.o from $$source.$(SRCEXT)";\
		$(CC) $(CPRODFLAGS) -c -o $(BUILDDIR)/$$source.o $(SRCDIR)/$$source.$(SRCEXT);\
	done
	@printf "\e[95m\e[1mLinking...\e[0m\n";
	@echo "  $(notdir $(OBJECTS))";
	@$(CC) $(CPRODFLAGS) -o $(TARGET) $(OBJECTS) main.cpp;