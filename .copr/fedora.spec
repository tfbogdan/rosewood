Name:       {{{ git_name }}}
Version:    {{{ git_version }}}
Release:    1%{?dist}
Summary:    C++ reflection framework
License:    MIT
URL:        https://github.com/tfbogdan/rosewood
VCS:        {{{ git_vcs }}}

Source: {{{ git_pack }}}

BuildRequires: cmake
BuildRequires: g++
BuildRequires: clang-devel
BuildRequires: llvm-devel
BuildRequires: gtest-devel
BuildRequires: fmt-devel
BuildRequires: ninja-build
BuildRequires: bison
BuildRequires: flex

%description
No nonsense C++ reflection framework built on clang tooling

%prep
{{{ git_setup_macro }}}

%changelog
{{{ git_changelog }}}

%build
cmake . -GNinja && ninja
