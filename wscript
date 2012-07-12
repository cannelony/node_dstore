#!/usr/bin/env python

def set_options(ctx):
  ctx.tool_options('compiler_cxx')

def configure(ctx):
  ctx.check_tool('compiler_cxx')
  ctx.check_tool('node_addon')

def build(ctx):
  obj = ctx.new_task_gen('cxx', 'shlib', 'node_addon')
  obj.cxxflags = [
    '-Wall',
    '-std=c++0x',
    '-DNDEBUG'
  ]
  obj.source = ['./src/node_dstore.cc', './src/DStore.cc', './src/Element.cc', './src/Dictionary.cc']
  obj.target = 'dstore'
