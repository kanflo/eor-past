# Component makefile for extras/past
#
# See examples/past for usage

INC_DIRS += $(ROOT)extras/past

# args for passing into compile rule generation
extras/past_INC_DIR =  $(ROOT)extras/past
extras/past_SRC_DIR =  $(ROOT)extras/past

$(eval $(call component_compile_rules,extras/past))
