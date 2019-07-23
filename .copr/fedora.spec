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
BuildRequires: make
BuildRequires: bison
BuildRequires: flex

Requires: llvm-libs
Requires: clang-libs

%description
No nonsense C++ reflection framework built on clang tooling

%prep
{{{ git_setup_macro }}}


%build

%cmake . -DCMAKE_BUILD_TYPE=Release -DBIN_INSTALL_DIR:PATH=%_bindir -DCMAKE_INSTALL_DIR:PATH=%_libdir/cmake
%make_build

%check
ctest -V %{?_smp_mflags}

%install
# cmake --build . --target install
%make_install

%files
%dir %_includedir/rosewood
%_includedir/rosewood/*.hpp

%dir %_libdir/cmake/rosewood
%_libdir/cmake/rosewood/*.cmake

%_bindir/rwc
%_libdir/librwruntime.a

%changelog
{{{ git_changelog }}}
