set MADYM_ROOT=
%MADYM_ROOT%madym_T1 --config madym_T1_config.txt --cwd test_dataset
%MADYM_ROOT%madym_AIF --config madym_AIF_config.txt --cwd test_dataset
%MADYM_ROOT%madym_DCE --config madym_ETM_auto_config.txt --cwd test_dataset
%MADYM_ROOT%madym_DCE --config madym_ETM_map_config.txt --cwd test_dataset
%MADYM_ROOT%madym_DCE --config madym_ETM_pop_config.txt --cwd test_dataset