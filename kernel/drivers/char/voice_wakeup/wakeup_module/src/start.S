/*
 * start.S
 *
 * Entry point of the Voice Wakeup Module.
 * The firmware code are executed in the L2Cache.
 *
 * Copyright (C) 2006 Ingenic Semiconductor Inc.
 *
 */
	.global interface
	.extern open
	.extern handler
	.extern close
	.extern cache_prefetch

	.extern set_handler
	.extern get_dma_address
	.extern ioctl
	.extern get_resource_addr
	.extern process_data
	.extern is_cpu_wakeup_by_dmic
	.extern set_sleep_buffer
	.extern get_sleep_process
	.extern set_dma_channel
	.extern voice_wakeup_enable
	.extern is_voice_wakeup_enabled
	.extern cpu_should_sleep
	.extern set_record_buffer
	.extern get_record_buffer
	.extern get_record_buffer_len
	.extern set_desc_addr

	.extern module_init
	.extern module_exit
	.section .text
	.type	interface, @function;
	.ent interface
interface:
	.word open
	.word handler
	.word close
	.word cache_prefetch
	.word set_handler
	.word get_dma_address
	.word ioctl
	.word get_resource_addr
	.word process_data
	.word is_cpu_wakeup_by_dmic
	.word set_sleep_buffer
	.word get_sleep_process
	.word set_dma_channel
	.word voice_wakeup_enable
	.word is_voice_wakeup_enabled
	.word cpu_should_sleep
	.word set_record_buffer
	.word get_record_buffer
	.word get_record_buffer_len
	.word set_desc_addr

	.word module_init
	.word module_exit
	.end  interface
