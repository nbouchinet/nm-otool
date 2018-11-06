/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   nm.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbouchin <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/10/16 10:34:00 by nbouchin          #+#    #+#             */
/*   Updated: 2018/11/06 09:43:33 by nbouchin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/libft_nm.h"

int		regular_files(t_mach_header_64 const *mach_header_64,
		t_fmetadata *fmetadata)
{
	if (is_fat(mach_header_64->magic))
		process_fat_header((t_fat_header*)mach_header_64, fmetadata);
	else
		process_header(mach_header_64, mach_header_64->magic, fmetadata);
	return (1);
}

int		archive_files(t_mach_header_64 const *mach_header_64,
		t_fmetadata *fmetadata)
{
	int					eof;
	int					is64;
	t_ar_hdr			*ar_hdr;

	eof = 0;
	is64 = 0;
	if (!ft_strncmp((char *)mach_header_64, ARMAG, SARMAG))
	{
		ar_hdr = (t_ar_hdr*)((char *)mach_header_64 + 8);
		while (42)
		{
			if (!ft_strncmp(ar_hdr->ar_fmag, ARFMAG, 2))
			{
				fmetadata->subfile = (char*)ar_hdr + sizeof(t_ar_hdr);
				if (((t_mach_header_64 *)((char *)ar_hdr + sizeof(t_ar_hdr) + ft_atoi((char *)ar_hdr->ar_name + 3)))->cputype == CPU_TYPE_X86_64)
				{
					is64 = 1;
					regular_files((t_mach_header_64 *)((char *)ar_hdr + sizeof(t_ar_hdr) + ft_atoi((char *)ar_hdr->ar_name + 3)), fmetadata);
				}
				ar_hdr = (t_ar_hdr*)((char *)ar_hdr + ft_atoi(ar_hdr->ar_size) + sizeof(t_ar_hdr));
				if (!ft_strncmp((char *)ar_hdr, ARMAG, SARMAG))
				{
					eof = 1;
					ar_hdr = (t_ar_hdr*)((char *)ar_hdr + 8);
				}
				if (is64 && eof)
					break ;
			}
			else
				break ;
		}
		return (1);
	}
	return (0);
}

void	init_file_metadata(t_fmetadata *fmetadata, int argc,
		char **argv, int nb_file)
{
	fmetadata->new_file = 1;
	fmetadata->fname = ft_strdup(argv[nb_file]);
	fmetadata->argc = argc;
	fmetadata->subfile = NULL;
	fmetadata->to_print = 1;
	fmetadata->alone = 0;
}

void	delete_file_metadata(t_fmetadata *fmetadata)
{
	free(fmetadata->fname);
	free(fmetadata);
}

void	print_error(char const *file_name, char const *error)
{
	ft_putstr_fd("nm: ", 2);
	ft_putstr_fd(file_name, 2);
	ft_putendl_fd(error, 2);
}

void	run_archive_files(t_mach_header_64 *mach_header_64,
		t_fmetadata *fmetadata)
{
	if (!archive_files(mach_header_64, fmetadata))
		print_error(fmetadata->fname,
				" The file was not recognized as a valide object file\n");
}

void	run_regular_files(t_mach_header_64 *mach_header_64,
		t_fmetadata *fmetadata)
{
	regular_files(mach_header_64, fmetadata);
}

void	file_error(int fd, t_stat buf, t_fmetadata *fmetadata)
{
	if (fstat(fd, &buf) == -1)
		print_error(fmetadata->fname, ": No such file or directory.");
	else if (S_ISDIR(buf.st_mode))
		print_error(fmetadata->fname, ": is a directory.");
	delete_file_metadata(fmetadata);
}

void	delete_data(t_mach_header_64 *mach_header_64,
		t_fmetadata *fmetadata, t_stat buf, int fd)
{
	delete_file_metadata(fmetadata);
	munmap(mach_header_64, buf.st_size);
	close(fd);
}

void	run_nm(t_mach_header_64 *mach_header_64, t_fmetadata *fmetadata,
		t_stat buf, int fd)
{
	if (is_magic(mach_header_64->magic))
		run_regular_files(mach_header_64, fmetadata);
	else
		run_archive_files(mach_header_64, fmetadata);
	delete_data(mach_header_64, fmetadata, buf, fd);
}

int		main(int argc, char **argv)
{
	int					nb_file;
	t_stat				buf;
	t_mach_header_64	*mach_header_64;
	t_fmetadata			*fmetadata;

	nb_file = 0;
	while (++nb_file < argc)
	{
		fmetadata = (t_fmetadata*)ft_memalloc(sizeof(t_fmetadata));
		fmetadata->fd = open(argv[nb_file], O_RDONLY);
		init_file_metadata(fmetadata, argc, argv, nb_file);
		if (fstat(fmetadata->fd, &buf) == -1 || S_ISDIR(buf.st_mode))
		{
			file_error(fmetadata->fd, buf, fmetadata);
			continue ;
		}
		mach_header_64 = mmap(NULL, buf.st_size,
		PROT_WRITE | PROT_READ, MAP_PRIVATE, fmetadata->fd, 0);
		if (mach_header_64 == MAP_FAILED)
		{
			delete_data(mach_header_64, fmetadata, buf, fmetadata->fd);
			continue ;
		}
		run_nm(mach_header_64, fmetadata, buf, fmetadata->fd);
	}
}
