/*
 * Copyright 2003 Ned Ludd <solar@gentoo.org>
 * Copyright 1999-2005 Gentoo Foundation
 * Distributed under the terms of the GNU General Public License v2
 * $Header: /var/cvsroot/gentoo-projects/pax-utils/paxelf.c,v 1.16 2005/04/07 00:22:42 vapier Exp $
 *
 ********************************************************************
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 ********************************************************************
 *
 * This program was written for the hcc suite by (solar|pappy)@g.o.
 * visit http://www.gentoo.org/proj/en/hardened/etdyn-ssp.xml for more
 * information on the Gentoo Hardened gcc suite
 * Also of interest is the pax site http://pax.grsecurity.net/
 * but you should know about that already.
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "paxelf.h"

char do_reverse_endian;

/*
 * Setup a bunch of helper functions to translate
 * binary defines into readable strings.
 */
#define QUERY(n) { #n, n }
typedef struct {
	const char *str;
	int value;
} pairtype;
static inline const char *find_pairtype(pairtype *pt, int type)
{
	int i;
	for (i = 0; pt[i].str; ++i)
		if (type == pt[i].value)
			return pt[i].str;
	return "UNKNOWN TYPE";
}

/* translate misc elf EI_ defines */
static pairtype elf_ei_class[] = {
	QUERY(ELFCLASSNONE),
	QUERY(ELFCLASS32),
	QUERY(ELFCLASS64),
	QUERY(ELFCLASSNUM),
	{ 0, 0 }
};
static pairtype elf_ei_data[] = {
	QUERY(ELFDATANONE),
	QUERY(ELFDATA2LSB),
	QUERY(ELFDATA2MSB),
	QUERY(ELFDATANUM),
	{ 0, 0 }
};
static pairtype elf_ei_version[] = {
	QUERY(EV_NONE),
	QUERY(EV_CURRENT),
	QUERY(EV_NUM),
	{ 0, 0 }
};
static pairtype elf_ei_osabi[] = {
	QUERY(ELFOSABI_NONE),
	QUERY(ELFOSABI_SYSV),
	QUERY(ELFOSABI_HPUX),
	QUERY(ELFOSABI_NETBSD),
	QUERY(ELFOSABI_LINUX),
	QUERY(ELFOSABI_SOLARIS),
	QUERY(ELFOSABI_AIX),
	QUERY(ELFOSABI_IRIX),
	QUERY(ELFOSABI_FREEBSD),
	QUERY(ELFOSABI_TRU64),
	QUERY(ELFOSABI_MODESTO),
	QUERY(ELFOSABI_OPENBSD),
	QUERY(ELFOSABI_ARM),
	QUERY(ELFOSABI_STANDALONE),
	{ 0, 0 }
};
const char *get_elfeitype(elfobj *elf, int ei_type, int type)
{
	switch (ei_type) {
		case EI_CLASS:   return find_pairtype(elf_ei_class, type);
		case EI_DATA:    return find_pairtype(elf_ei_data, type);
		case EI_VERSION: return find_pairtype(elf_ei_version, type);
		case EI_OSABI:   return find_pairtype(elf_ei_osabi, type);
	}
	return "UNKNOWN EI TYPE";
}

/* translate elf ET_ defines */
static pairtype elf_etypes[] = {
	QUERY(ET_NONE),
	QUERY(ET_REL),
	QUERY(ET_EXEC),
	QUERY(ET_DYN),
	QUERY(ET_CORE),
	QUERY(ET_NUM),
	QUERY(ET_LOOS),
	QUERY(ET_HIOS),
	QUERY(ET_LOPROC),
	QUERY(ET_HIPROC),
	{ 0, 0 }
};
const char *get_elfetype(elfobj *elf)
{
	int type;
	if (elf->elf_class == ELFCLASS32)
		type = EGET(EHDR32(elf->ehdr)->e_type);
	else
		type = EGET(EHDR64(elf->ehdr)->e_type);
	return find_pairtype(elf_etypes, type);
}

/* translate elf EM_ defines */
static pairtype elf_emtypes[] = {
	QUERY(EM_NONE),
	QUERY(EM_M32),
	QUERY(EM_SPARC),
	QUERY(EM_386),
	QUERY(EM_68K),
	QUERY(EM_88K),
	QUERY(EM_860),
	QUERY(EM_MIPS),
	QUERY(EM_S370),
	QUERY(EM_MIPS_RS3_LE),
	QUERY(EM_PARISC),
	QUERY(EM_VPP500),
	QUERY(EM_SPARC32PLUS),
	QUERY(EM_960),
	QUERY(EM_PPC),
	QUERY(EM_PPC64),
	QUERY(EM_S390),
	QUERY(EM_V800),
	QUERY(EM_FR20),
	QUERY(EM_RH32),
	QUERY(EM_RCE),
	QUERY(EM_ARM),
	QUERY(EM_FAKE_ALPHA),
	QUERY(EM_SH),
	QUERY(EM_SPARCV9),
	QUERY(EM_TRICORE),
	QUERY(EM_ARC),
	QUERY(EM_H8_300),
	QUERY(EM_H8_300H),
	QUERY(EM_H8S),
	QUERY(EM_H8_500),
	QUERY(EM_IA_64),
	QUERY(EM_MIPS_X),
	QUERY(EM_COLDFIRE),
	QUERY(EM_68HC12),
	QUERY(EM_MMA),
	QUERY(EM_PCP),
	QUERY(EM_NCPU),
	QUERY(EM_NDR1),
	QUERY(EM_STARCORE),
	QUERY(EM_ME16),
	QUERY(EM_ST100),
	QUERY(EM_TINYJ),
	QUERY(EM_X86_64),
	QUERY(EM_PDSP),
	QUERY(EM_FX66),
	QUERY(EM_ST9PLUS),
	QUERY(EM_ST7),
	QUERY(EM_68HC16),
	QUERY(EM_68HC11),
	QUERY(EM_68HC08),
	QUERY(EM_68HC05),
	QUERY(EM_SVX),
	QUERY(EM_ST19),
	QUERY(EM_VAX),
	QUERY(EM_CRIS),
	QUERY(EM_JAVELIN),
	QUERY(EM_FIREPATH),
	QUERY(EM_ZSP),
	QUERY(EM_MMIX),
	QUERY(EM_HUANY),
	QUERY(EM_PRISM),
	QUERY(EM_AVR),
	QUERY(EM_FR30),
	QUERY(EM_D10V),
	QUERY(EM_D30V),
	QUERY(EM_V850),
	QUERY(EM_M32R),
	QUERY(EM_MN10300),
	QUERY(EM_MN10200),
	QUERY(EM_PJ),
	QUERY(EM_OPENRISC),
	QUERY(EM_ARC_A5),
	QUERY(EM_XTENSA),
	QUERY(EM_NUM),
	QUERY(EM_ALPHA),
	{ 0, 0 }
};
const char *get_elfemtype(int type)
{
	return find_pairtype(elf_emtypes, type);
}

/* translate elf PT_ defines */
static pairtype elf_ptypes[] = {
	QUERY(PT_NULL),
	QUERY(PT_LOAD),
	QUERY(PT_DYNAMIC),
	QUERY(PT_INTERP),
	QUERY(PT_NOTE),
	QUERY(PT_SHLIB),
	QUERY(PT_PHDR),
	QUERY(PT_TLS),
	QUERY(PT_GNU_EH_FRAME),
	QUERY(PT_GNU_STACK),
	QUERY(PT_GNU_RELRO),
	QUERY(PT_PAX_FLAGS),
	{ 0, 0 }
};
const char *get_elfptype(int type)
{
	return find_pairtype(elf_ptypes, type);
}

/* translate elf PT_ defines */
static pairtype elf_dtypes[] = {
	QUERY(DT_NULL),
	QUERY(DT_NEEDED),
	QUERY(DT_PLTRELSZ),
	QUERY(DT_PLTGOT),
	QUERY(DT_HASH),
	QUERY(DT_STRTAB),
	QUERY(DT_SYMTAB),
	QUERY(DT_RELA),
	QUERY(DT_RELASZ),
	QUERY(DT_RELAENT),
	QUERY(DT_STRSZ),
	QUERY(DT_SYMENT),
	QUERY(DT_INIT),
	QUERY(DT_FINI),
	QUERY(DT_SONAME),
	QUERY(DT_RPATH),
	QUERY(DT_SYMBOLIC),
	QUERY(DT_REL),
	QUERY(DT_RELSZ),
	QUERY(DT_RELENT),
	QUERY(DT_PLTREL),
	QUERY(DT_DEBUG),
	QUERY(DT_TEXTREL),
	QUERY(DT_JMPREL),
	QUERY(DT_BIND_NOW),
	QUERY(DT_INIT_ARRAY),
	QUERY(DT_FINI_ARRAY),
	QUERY(DT_INIT_ARRAYSZ),
	QUERY(DT_FINI_ARRAYSZ),
	QUERY(DT_RUNPATH),
	QUERY(DT_FLAGS),
	QUERY(DT_ENCODING),
	QUERY(DT_PREINIT_ARRAY),
	QUERY(DT_PREINIT_ARRAYSZ),
	QUERY(DT_NUM),
	{ 0, 0 }
};
const char *get_elfdtype(int type)
{
	return find_pairtype(elf_dtypes, type);
}

/* translate elf STT_ defines */
static pairtype elf_stttypes[] = {
	QUERY(STT_NOTYPE),
	QUERY(STT_OBJECT),
	QUERY(STT_FUNC),
	QUERY(STT_SECTION),
	QUERY(STT_FILE),
	QUERY(STT_LOPROC),
	QUERY(STT_HIPROC),
	QUERY(STB_LOCAL),
	QUERY(STB_GLOBAL),
	QUERY(STB_WEAK),
	QUERY(STB_LOPROC),
	QUERY(STB_HIPROC),
	{ 0, 0 }
};
const char *get_elfstttype(int type)
{
	return find_pairtype(elf_stttypes, type & 0xF);
}

/* Read an ELF into memory */
#define IS_ELF_BUFFER(buff) \
	(buff[EI_MAG0] == ELFMAG0 && \
	 buff[EI_MAG1] == ELFMAG1 && \
	 buff[EI_MAG2] == ELFMAG2 && \
	 buff[EI_MAG3] == ELFMAG3)
#define DO_WE_LIKE_ELF(buff) \
	((buff[EI_CLASS] == ELFCLASS32 || buff[EI_CLASS] == ELFCLASS64) && \
	 (buff[EI_DATA] == ELFDATA2LSB || buff[EI_DATA] == ELFDATA2MSB) && \
	 (buff[EI_VERSION] == EV_CURRENT) && \
	 (buff[EI_OSABI] == ELFOSABI_NONE || buff[EI_OSABI] == ELFOSABI_LINUX))
elfobj *readelf(const char *filename)
{
	struct stat st;
	int fd;
	elfobj *elf;

	if (stat(filename, &st) == -1)
		return NULL;

	if ((fd = open(filename, O_RDONLY)) == -1)
		return NULL;

	/* make sure we have enough bytes to scan e_ident */
	if (st.st_size <= EI_NIDENT)
		goto close_fd_and_return;

	elf = (elfobj*)malloc(sizeof(*elf));
	if (elf == NULL)
		goto close_fd_and_return;
	memset(elf, 0x00, sizeof(*elf));

	elf->fd = fd;
	elf->len = st.st_size;
	elf->data = (char *) mmap(0, elf->len, PROT_READ, MAP_PRIVATE, fd, 0);
	if (elf->data == (char *) MAP_FAILED)
		goto free_elf_and_return;

	if (!IS_ELF_BUFFER(elf->data)) /* make sure we have an elf */
		goto unmap_data_and_return;
	if (!DO_WE_LIKE_ELF(elf->data)) /* check class and stuff */
		goto unmap_data_and_return;

	elf->elf_class = elf->data[EI_CLASS];
	do_reverse_endian = (ELF_DATA != elf->data[EI_DATA]);
	elf->ehdr = (void *)elf->data;
	if (elf->elf_class == ELFCLASS32) {
		Elf32_Ehdr *ehdr = EHDR32(elf->ehdr);
		elf->phdr = ehdr->e_phoff ? elf->data + EGET(ehdr->e_phoff) : 0;
		elf->shdr = ehdr->e_shoff ? elf->data + EGET(ehdr->e_shoff) : 0;
	} else {
		Elf64_Ehdr *ehdr = EHDR64(elf->ehdr);
		elf->phdr = ehdr->e_phoff ? elf->data + EGET(ehdr->e_phoff) : 0;
		elf->shdr = ehdr->e_shoff ? elf->data + EGET(ehdr->e_shoff) : 0;
	}

	return elf;

unmap_data_and_return:
	munmap(elf->data, elf->len);
free_elf_and_return:
	free(elf);
close_fd_and_return:
	close(fd);
	return NULL;
}

/* undo the readelf() stuff */
void unreadelf(elfobj *elf)
{
	munmap(elf->data, elf->len);
	close(elf->fd);
	free(elf);
}

/* the display logic is:
 * lower case: explicitly disabled
 * upper case: explicitly enabled
 * - : default */
char *pax_short_hf_flags(unsigned long flags)
{
	static char buffer[7];

	buffer[0] = (flags & HF_PAX_PAGEEXEC ? 'p' : 'P');
	buffer[1] = (flags & HF_PAX_EMUTRAMP ? 'E' : 'e');
	buffer[2] = (flags & HF_PAX_MPROTECT ? 'm' : 'M');
	buffer[3] = (flags & HF_PAX_RANDMMAP ? 'r' : 'R');
	buffer[4] = (flags & HF_PAX_RANDEXEC ? 'X' : 'x');
	buffer[5] = (flags & HF_PAX_SEGMEXEC ? 's' : 'S');
	buffer[6] = 0;

	return buffer;
}
char *pax_short_pf_flags(unsigned long flags)
{
	static char buffer[13];

	buffer[0] = (flags & PF_PAGEEXEC ? 'P' : '-');
	buffer[1] = (flags & PF_NOPAGEEXEC ? 'p' : '-');
	buffer[2] = (flags & PF_SEGMEXEC ? 'S' : '-');
	buffer[3] = (flags & PF_NOSEGMEXEC ? 's' : '-');
	buffer[4] = (flags & PF_MPROTECT ? 'M' : '-');
	buffer[5] = (flags & PF_NOMPROTECT ? 'm' : '-');
	buffer[6] = (flags & PF_RANDEXEC ? 'X' : '-');
	buffer[7] = (flags & PF_NORANDEXEC ? 'x' : '-');
	buffer[8] = (flags & PF_EMUTRAMP ? 'E' : '-');
	buffer[9] = (flags & PF_NOEMUTRAMP ? 'e' : '-');
	buffer[10] = (flags & PF_RANDMMAP ? 'R' : '-');
	buffer[11] = (flags & PF_NORANDMMAP ? 'r' : '-');
	buffer[12] = 0;

	return buffer;
}

char *gnu_short_stack_flags(unsigned long flags)
{
	static char buffer[4];

	buffer[0] = (flags & PF_R ? 'R' : '-');
	buffer[1] = (flags & PF_W ? 'W' : '-');
	buffer[2] = (flags & PF_X ? 'X' : '-');
	buffer[3] = 0;

	return buffer;
}

void *elf_findsecbyname(elfobj *elf, const char *name)
{
	int i;
	char *shdr_name;
#define FINDSEC(B) \
	if (elf->elf_class == ELFCLASS ## B) { \
	Elf ## B ## _Ehdr *ehdr = EHDR ## B (elf->ehdr); \
	Elf ## B ## _Shdr *shdr = SHDR ## B (elf->shdr); \
	Elf ## B ## _Shdr *strtbl = &(shdr[EGET(ehdr->e_shstrndx)]); \
	for (i = 0; i < EGET(ehdr->e_shnum); ++i) { \
		shdr_name = (char *) (elf->data + EGET(strtbl->sh_offset) + EGET(shdr[i].sh_name)); \
		if (!strcmp(shdr_name, name)) \
			return &(shdr[i]); \
	} }
	FINDSEC(32)
	FINDSEC(64)
	return NULL;
}
