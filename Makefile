BLACK		=	$(shell tput setaf 0)
RED			=	$(shell tput setaf 1)
GREEN		=	$(shell tput setaf 2)
YELLOW		=	$(shell tput setaf 3)
BLUE		=	$(shell tput setaf 4)
MAGENTA		=	$(shell tput setaf 5)
CYAN		=	$(shell tput setaf 6)
WHITE		=	$(shell tput setaf 7)
BOLD        =	$(shell tput bold)
UNDERLINE   =	$(shell tput smul)
REVERSE     =	$(shell tput rev)
BLINK       =	$(shell tput blink)
DIM         =	$(shell tput dim)
STANDOUT    =	$(shell tput smso)
RESET		=	$(shell tput sgr0)

CC 			=	c++
CFLAGS		=	-Wall -Werror -Wextra -std=c++98 -I include
MANDATORY 	=	src/main.cpp \
				src/ServerConfig.cpp \
				src/ConfigParser.cpp \
				src/SignalHandler.cpp \
				src/Server.cpp \
				src/Client.cpp \
				src/HttpParser.cpp \
				# src/HttpRequest.cpp
SRC 		=	$(MANDATORY)
OBJS_DIR	=	objs/
OBJS		=	$(SRC:%.cpp=$(OBJS_DIR)%.o)

NAME 		=	webserv

all: $(NAME)

$(OBJS_DIR)%.o: %.cpp
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

$(NAME): $(OBJS)
	@echo "$(YELLOW)$(BOLD)$(UNDERLINE) ****/compiling $(NAME)\****$(RESET)"
	@$(CC) $(CFLAGS) -o $(NAME) $(OBJS)
 # -OFast -funroll-loops -finline-functions
	@echo "$(GREEN)$(BOLD)$(REVERSE) ****\\\$(NAME) compiled!/****$(RESET)"

#siege:
#	siege -c 10 -t 1M http://localhost:8080

clean:
	@rm -rf $(OBJS)
	@rm -rf $(OBJS_DIR)
	@echo "$(RED)$(BOLD)$(DIM) --->objects deleted<----$(RESET)"

fclean: clean
	@rm -f $(NAME)
	@echo "$(RED)$(BOLD)$(REVERSE) ---->$(NAME) deleted<----$(RESET)"

re: fclean all

.PHONY: all clean fclean re