/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   process_header.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbouchin <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/10/17 16:49:09 by nbouchin          #+#    #+#             */
/*   Updated: 2018/11/08 14:33:37 by nbouchin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/libft_nm.h"

void	get_metadata_64(t_mach_header_64 const *mach_header_64)
{
	uint32_t			ncmds;
	t_load_command		*lcommand;
	t_metadata			*mdata;

	ncmds = mach_header_64->ncmds;
	mdata = (t_metadata*)ft_memalloc(sizeof(t_metadata));
	lcommand = (t_load_command*)(mach_header_64 + 1);
	while (--ncmds)
	{
		if (lcommand->cmd == LC_SEGMENT_64)
			mdata->sectab = otool_get_section(lcommand, mach_header_64, mdata);
		else if (lcommand->cmd == LC_SYMTAB)
			(mdata->symtab) ? free(mdata->symtab) : 0;
		lcommand = (t_load_command*)((char*)lcommand + lcommand->cmdsize);
	}
	free(mdata);
}

void	get_big_metadata_32(t_mach_header_64 const *mach_header_64)
{
	uint32_t			ncmds;
	t_load_command		*lcommand;
	t_metadata			*metadata;

	ncmds = swi(mach_header_64->ncmds);
	metadata = (t_metadata*)ft_memalloc(sizeof(t_metadata));
	lcommand = (t_load_command*)((t_mach_header*)mach_header_64 + 1);
	while (--ncmds)
	{
		if (swi(lcommand->cmd) == LC_SEGMENT)
			metadata->sectab =
				otool_get_section(lcommand, mach_header_64, metadata);
		else if (swi(lcommand->cmd) == LC_SYMTAB)
			(metadata->symtab) ? free(metadata->symtab) : 0;
		lcommand = (t_load_command*)((char*)lcommand
				+ swi(lcommand->cmdsize));
	}
	free(metadata);
}

void	get_metadata_32(t_mach_header_64 const *mach_header_64)
{
	uint32_t			ncmds;
	t_load_command		*lcommand;
	t_metadata			*mdata;

	ncmds = mach_header_64->ncmds;
	mdata = (t_metadata*)ft_memalloc(sizeof(t_metadata));
	lcommand = (t_load_command*)((t_mach_header*)mach_header_64 + 1);
	while (--ncmds)
	{
		if (lcommand->cmd == LC_SEGMENT)
			mdata->sectab = otool_get_section(lcommand, mach_header_64, mdata);
		else if (lcommand->cmd == LC_SYMTAB)
			(mdata->symtab) ? free(mdata->symtab) : 0;
		lcommand = (t_load_command*)((char*)lcommand + lcommand->cmdsize);
	}
	free(mdata);
}

void	print_cputype(t_mach_header_64 const *mach_header_64, int pass,
		t_fmetadata *fmetadata)
{
	if (fmetadata->subfile)
	{
		ft_printf("%s(%s):\n", fmetadata->fname, fmetadata->subfile);
		return ;
	}
	if (swi(mach_header_64->cputype)
			== CPU_TYPE_POWERPC && pass == 2)
		ft_printf("%s (architecture ppc):\n", fmetadata->fname);
	else if (mach_header_64->cputype == CPU_TYPE_I386 && pass == 2
			&& fmetadata->alone > 1)
		ft_printf("%s (architecture i386):\n", fmetadata->fname);
	else if (mach_header_64->cputype == CPU_TYPE_I386 && pass == 2
			&& fmetadata->alone == 1)
		ft_printf("%s:\n", fmetadata->fname);
	else if (swi(mach_header_64->cputype) == CPU_TYPE_POWERPC)
		ft_printf("%s:\n", fmetadata->fname);
	else if (mach_header_64->cputype == CPU_TYPE_I386)
		ft_printf("%s:\n", fmetadata->fname);
	else
		ft_printf("%s:\n", fmetadata->fname);
}

int		process_header(t_mach_header_64 const *mach_header_64,
		uint32_t const magic, t_fmetadata *fmetadata)
{
	static int			pass = 0;

	if (fmetadata->new_file == 1)
	{
		pass = 0;
		fmetadata->new_file = 0;
	}
	if (is_64bits(magic))
	{
		pass = 1;
		print_cputype(mach_header_64, pass, fmetadata);
		get_metadata_64(mach_header_64);
	}
	if ((is_32bits(magic) && (pass == 0 || pass == 2)))
	{
		pass = 2;
		print_cputype(mach_header_64, pass, fmetadata);
		if (mach_header_64->magic == MH_CIGAM)
			get_big_metadata_32(mach_header_64);
		else
			get_metadata_32(mach_header_64);
	}
	return (1);
}
