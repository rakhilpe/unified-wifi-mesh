##########################################################################
 # Copyright 2023 Comcast Cable Communications Management, LLC
 #
 # Licensed under the Apache License, Version 2.0 (the "License");
 # you may not use this file except in compliance with the License.
 # You may obtain a copy of the License at
 #
 # http://www.apache.org/licenses/LICENSE-2.0
 #
 # Unless required by applicable law or agreed to in writing, software
 # distributed under the License is distributed on an "AS IS" BASIS,
 # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 # See the License for the specific language governing permissions and
 # limitations under the License.
 #
 # SPDX-License-Identifier: Apache-2.0
##########################################################################

OS = $(shell uname)
PLATFORM = $(shell uname -a)


BASE = $(shell pwd)

ONEWIFI_EM_HOME = $(BASE)/../..
ONEWIFI_HOME = $(BASE)/../../../one_wifi_program
ONEWIFI_HAL_INTF_HOME = $(BASE)/../../../one_wifi_program/halinterface
ONEWIFI_BUS_LIB_HOME = $(ONEWIFI_HOME)/generic/lib/bus
ONEWIFI_EM_SRC = $(ONEWIFI_EM_HOME)/src
INSTALLDIR = $(ONEWIFI_EM_HOME)/install
OBJDIR = obj
CXX = g++
CC = gcc
AR= ar cr
RM = -rm -rf