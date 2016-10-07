all:
	@echo "\033[31m\ncompiling lib....\033[0m"
	cd ./compiler/lib && make
	@echo "\033[31mcompiling lib successful....\033[0m"
	@echo "\033[31m\ncompiling lex....\033[0m"
	cd ./compiler/lex && make
	@echo "\033[31mcompiling lex successful....\033[0m"
	@echo "\033[31m\ncompiling llama....\033[0m"
	cd ./compiler/parser && make bin=llama
	@echo "\033[31mcompiling llama successful....\033[0m"

clean:
	@echo "\033[31m\ncleaning lib....\033[0m"
	cd ./compiler/lib && make clean
	@echo "\033[31mcleaning lib successful....\033[0m"
	@echo "\033[31m\ncleaning lex....\033[0m"
	cd ./compiler/lex && make clean
	@echo "\033[31mcleaning lex successful....\033[0m"
	@echo "\033[31m\ncleaning parser....\033[0m"
	cd ./compiler/parser && make clean
	@echo "\033[31mcleaning parser successful....\033[0m"