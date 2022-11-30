For pre commit hooks install the following. Note that clang-tidy is only available on linux. As such the commit hook can only be run on linux.
If using linux, uncomment line 9 to use clang-tidy.

```bash
brew install pre-commit
brew install llvm
pre-commit install
pre-commit run --all-files
```

After running the installation. The hooks will run every commit.