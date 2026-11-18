"""检查桌面应用开发所需的工具链。运行：python script/develop_env_check.py"""

import platform
import shutil
import subprocess
import sys


def run(executable, *args, input_text=None):
    try:
        result = subprocess.run(
            [executable, *args],
            input=input_text,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            timeout=10,
            check=False,
        )
    except (OSError, subprocess.TimeoutExpired) as exc:
        return None, str(exc)
    return result.returncode, result.stdout + result.stderr


def check_tool(name, version_arg="--version"):
    path = shutil.which(name)
    if path is None:
        print(f"[缺失] {name}：未在 PATH 中找到")
        return False
    code, output = run(path, version_arg)
    if code != 0:
        print(f"[失败] {name}：无法运行（{output.strip()}）")
        return False
    version = output.splitlines()[0] if output.splitlines() else "版本未知"
    print(f"[通过] {name}：{version}（{path}）")
    return True


def check_windows_gcc():
    path = shutil.which("gcc")
    if path is None:
        print("[缺失] GCC：未在 PATH 中找到 MinGW-w64 GCC")
        return False

    code, details = run(path, "-v")
    if code != 0:
        print(f"[失败] GCC：无法读取编译器配置（{details.strip()}）")
        return False

    target = next((line.split(":", 1)[1].strip() for line in details.splitlines()
                   if line.startswith("Target:")), "未知")
    thread = next((line.split(":", 1)[1].strip().lower() for line in details.splitlines()
                   if line.startswith("Thread model:")), "未知")
    version = next((line.strip() for line in details.splitlines()
                    if line.startswith("gcc version ")), "版本未知")

    # _UCRT 由 MinGW-w64 的目标头文件定义，比安装目录名称更可靠。
    macro_code, macros = run(path, "-dM", "-E", "-x", "c", "-",
                             input_text="#include <_mingw.h>\n")
    ucrt = macro_code == 0 and any(
        line.strip().startswith("#define _UCRT ") or line.strip() == "#define _UCRT"
        for line in macros.splitlines()
    )
    target_ok = target == "x86_64-w64-mingw32"
    thread_ok = thread == "posix"
    print(f"[{'通过' if target_ok else '不符'}] GCC 目标：{target}（要求 x86_64-w64-mingw32）")
    print(f"[{'通过' if thread_ok else '不符'}] 线程模型：{thread}（要求 POSIX）")
    print(f"[{'通过' if ucrt else '不符'}] C 运行时：{'UCRT' if ucrt else '未检测到 UCRT'}（要求 UCRT）")
    print(f"       编译器：{version}（{path}）")
    return target_ok and thread_ok and ucrt


def main():
    print(f"开发环境检查（{platform.system()}）")
    results = [check_tool("cmake"), check_tool("git")]
    if platform.system() == "Windows":
        results.append(check_windows_gcc())
    if all(results):
        print("检查完成：环境满足要求。")
        return 0
    print("检查完成：环境不满足要求，请安装或调整上述工具。")
    return 1


if __name__ == "__main__":
    sys.exit(main())
