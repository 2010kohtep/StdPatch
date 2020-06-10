#pragma once

unsigned int InstructionLength(unsigned char *pc);

template <typename T> unsigned int InstructionLength(T *pc)
{
	return InstructionLength((unsigned char *)pc);
}