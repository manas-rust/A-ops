Name:		A-Ops
Version:	v1.1.1
Release:	1
Summary:	The intelligent ops toolkit for openEuler
License:	MulanPSL2
URL:		https://gitee.com/openeuler/A-Ops
Source0:	%{name}-%{version}.tar.gz
# Source1:	A-Ops-web-node-modules.tar.gz
# patch0001:	0001-fix-diag-return.patch


# build for gopher
BuildRequires:	cmake gcc-c++ yum elfutils-devel clang >= 10.0.1 llvm libconfig-devel
BuildRequires:	libmicrohttpd-devel

# build for ragdoll & aops basic module
BuildRequires:  python3-setuptools  python3-werkzeug python3-libyang
BuildRequires:	git python3-devel systemd

# build for spider & aops basic module

# build for web
BuildRequires: nodejs  nodejs-yarn

%description
The intelligent ops toolkit for openEuler


%package -n aops-utils
Summary:    utils for A-Ops
Requires:   python3-concurrent-log-handler python3-xmltodict python3-pyyaml python3-marshmallow >= 3.13.0
Requires:   python3-requests python3-xlrd python3-prettytable python3-pygments

%description -n aops-utils
utils for A-Ops


%package -n aops-cli
Summary:        cli of A-ops
Requires: 	aops-utils = %{version}-%{release}

%description -n aops-cli
commandline tool of aops, offer commands for account management, host management,
host group management, task and template management of ansible.


%package -n aops-manager
Summary:    manager of A-ops
Requires:   aops-utils = %{version}-%{release} ansible >= 2.9.0
Requires:   python3-pyyaml python3-marshmallow >= 3.13.0 python3-flask python3-flask-restful
Requires:   python3-requests sshpass python3-uWSGI

%description -n aops-manager
manager of A-ops, support software deployment and installation, account management, host management,
host group management, task and template management of ansible.


%package -n aops-database
Summary:    database center of A-ops
Requires:   aops-utils = %{version}-%{release} python3-pyyaml
Requires:   python3-elasticsearch >= 7 python3-requests python3-werkzeug python3-urllib3
Requires:   python3-flask python3-flask-restful python3-PyMySQL python3-sqlalchemy
Requires:   python3-prometheus-api-client python3-uWSGI

%description -n aops-database
database center of A-ops, offer database proxy of mysql, elasticsearch and prometheus time series database.


%package -n aops-web
Summary:    website for A-Ops
Requires:   nginx

%description -n aops-web
website for A-Ops, deployed by Nginx


%define debug_package %{nil}

%prep
%autosetup -p1 -n %{name}-%{version}
# setup -T -D -a 1
#patch0001 -p1
#cp -r A-Ops-web-node-modules/node_modules aops-web/

%build
# build for aops-utils
pushd aops-utils
%py3_build
popd

#build for aops-cli
pushd aops-cli
%py3_build
popd

#build for aops-manager
pushd aops-manager
%py3_build
popd

#build for aops-database
pushd aops-database
%py3_build
popd


#build for aops-web
pushd aops-web
yarn install
yarn build
popd


%install
# install for utils
pushd aops-utils
%py3_install
popd

# install for cli
pushd aops-cli
%py3_install
popd

# install for manager
pushd aops-manager
%py3_install
mkdir -p %{buildroot}/%{python3_sitelib}/aops_manager/deploy_manager/ansible_handler
cp -r aops_manager/deploy_manager/ansible_handler/* %{buildroot}/%{python3_sitelib}/aops_manager/deploy_manager/ansible_handler
mkdir -p %{buildroot}/%{python3_sitelib}/aops_manager/deploy_manager/tasks
cp -r aops_manager/deploy_manager/tasks/* %{buildroot}/%{python3_sitelib}/aops_manager/deploy_manager/tasks
popd

# install for database
pushd aops-database
%py3_install
popd

# install for web
pushd aops-web
mkdir -p %{buildroot}/opt/aops_web
cp -r dist %{buildroot}/opt/aops_web/
mkdir -p %{buildroot}/%{_sysconfdir}/nginx
cp -r deploy/aops-nginx.conf %{buildroot}/%{_sysconfdir}/nginx/
mkdir -p %{buildroot}/usr/lib/systemd/system
cp -r deploy/aops-web.service %{buildroot}/usr/lib/systemd/system/
popd


%files -n aops-utils
%doc README.*
%attr(0644,root,root) %{_sysconfdir}/aops/system.ini
%{python3_sitelib}/aops_utils*.egg-info
%{python3_sitelib}/aops_utils/*
%attr(0755,root,root) %{_bindir}/aops-utils


%files -n aops-cli
%attr(0755,root,root) %{_bindir}/aops
%{python3_sitelib}/aops_cli*.egg-info
%{python3_sitelib}/aops_cli/*


%files -n aops-manager
%attr(0644,root,root) %{_sysconfdir}/aops/manager.ini
%attr(0755,root,root) %{_bindir}/aops-manager
%attr(0755,root,root) %{_unitdir}/aops-manager.service
%{python3_sitelib}/aops_manager*.egg-info
%{python3_sitelib}/aops_manager/*


%files -n aops-database
%attr(0644,root,root) %{_sysconfdir}/aops/database.ini
%attr(0644,root,root) %{_sysconfdir}/aops/default.json
%attr(0755,root,root) %{_unitdir}/aops-database.service
%attr(0755,root,root) %{_bindir}/aops-database
%attr(0755,root,root) %{_bindir}/aops-basedatabase
%{python3_sitelib}/aops_database*.egg-info
%{python3_sitelib}/aops_database/*


%files -n aops-web
%attr(0755, root, root) /opt/aops_web/dist/*
%attr(0755, root, root) %{_sysconfdir}/nginx/aops-nginx.conf
%attr(0755, root, root) %{_unitdir}/aops-web.service


%changelog
