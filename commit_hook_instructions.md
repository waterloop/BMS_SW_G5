For pre commit hooks install the following. Note that clang-tidy is only readily available on linux. Clang-tidy has to be built manually in other OS.
If using linux, uncomment line 9 to use clang-tidy.

```bash
brew install pre-commit
brew install llvm
pre-commit install
pre-commit run --all-files
```

After running the installation. The hooks will run every commit.

List of resources can be found below:
https://pre-commit.com/
https://clang.llvm.org/extra/clang-tidy/
https://clang.llvm.org/docs/HowToSetupToolingForLLVM.html
https://github.com/pocc/pre-commit-hooks
https://www.kdab.com/clang-tidy-part-1-modernize-source-code-using-c11c14/
https://stackoverflow.com/questions/47255526/how-to-build-the-latest-clang-tidy