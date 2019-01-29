# other-shell

# Simple shell for Unix

Commands:
- `cd`: change current working directory
- `exit`: terminate the shell
- `jobs`: list all current jobs
- `fg`: put a job to foreground
- `bg`: put a job to background

Features:

- Run programs in foreground or background (`&`)
- `Ctrl-C` signal handler
- Standard input/output redirection operators: `<` and `>`
- Create pipeline using operator: `|`
- Job control support: `Ctrl-Z`, `jobs`, `fg`, and `bg`
