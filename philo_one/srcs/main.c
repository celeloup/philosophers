/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: celeloup <celeloup@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/12 14:33:09 by celeloup          #+#    #+#             */
/*   Updated: 2021/03/16 20:20:03 by celeloup         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../philo_one.h"

void	message(int philosopher, int type, uint64_t start_time, pthread_mutex_t write_lock)
{
	pthread_mutex_lock(&write_lock);
	if (type == FORK)
		printf("%" PRIu64 " %d has taken a fork\n", get_time() \
			- start_time, philosopher);
	else if (type == EAT)
		printf("%" PRIu64 " %d is eating\n", get_time() \
			- start_time, philosopher);
	else if (type == SLEEP)
		printf("%" PRIu64 " %d is sleeping\n", get_time() \
			- start_time, philosopher);
	else if (type == THINK)
		printf("%" PRIu64 " %d is thinking\n", get_time() \
			- start_time, philosopher);
	else
		printf("%" PRIu64 " %d is dead\n", get_time() \
			- start_time, philosopher);
	if (type != DIE)
		pthread_mutex_unlock(&write_lock);
}

int		parser(t_params *param, char **args)
{
	param->nb_philo = ft_atoi(args[1]);
	param->time_die = ft_atoi(args[2]);
	param->time_eat = ft_atoi(args[3]);
	param->time_sleep = ft_atoi(args[4]);
	if (args[5])
		param->nb_meal = ft_atoi(args[5]);
	else
		param->nb_meal = -1;
	if (!param->nb_philo || !param->time_die || !param->time_eat \
		|| !param->time_sleep || !param->nb_meal)
	{
		printf("Error: wrong argument.\n");
		return (1);
	}
	param->start_time = 0;
	pthread_mutex_init(&(param->write_lock), NULL);
	return (0);
}

int		usage(char *programme)
{
	printf("usage: %s nb_philo time_die time_eat time_sleep [nb_meal]\n\
	nb_philo: number of philosophers (and forks)\n\
	time_die: (in ms) time before a philosopher starves to death\n\
	time_eat: (in ms) time a philosopher takes to eat\n\
	time_sleep: (in ms) time a philosopher takes to sleep\n\
	[nb_meal]: number of meals a philosopher must eat [optional]\n", programme);
	return (EXIT_FAILURE);
}

void	*philosophizing(void *args)
{
	t_philo		*self;
	int			i;
	uint64_t	done_activity;
	int			left;
	int			right;

	self = (t_philo *)args;
	while (self->params->start_time == 0)
		usleep(50);
	left = self->id - 1;
	right = (left + 1) % self->params->nb_philo;
	i = 1;
	while (i < self->params->nb_meal + 1)
	{
		if (left < right)
		{
			pthread_mutex_lock(&(*self->forks)[left]);
			pthread_mutex_lock(&(*self->forks)[right]);
		}
		else
		{
			pthread_mutex_lock(&(*self->forks)[right]);
			pthread_mutex_lock(&(*self->forks)[left]);
		}
		message(self->id, FORK, self->params->start_time, self->params->write_lock);
		message(self->id, FORK, self->params->start_time, self->params->write_lock);
		done_activity = get_time() + self->params->time_eat;
		message(self->id, EAT, self->params->start_time, self->params->write_lock);
		self->time_death = get_time() + self->params->time_die;
		while (done_activity > get_time())
			usleep(50);
		pthread_mutex_unlock(&(*self->forks)[left]);
		pthread_mutex_unlock(&(*self->forks)[right]);
		message(self->id, SLEEP, self->params->start_time, self->params->write_lock);
		done_activity = get_time() + self->params->time_sleep;
		while (done_activity > get_time())
			usleep(50);
		message(self->id, THINK, self->params->start_time, self->params->write_lock);
		if (self->nb_meal != -1)
			i++;
	}
	return (NULL);
}

void	initialisation(t_params *parameters, t_philo **philosophers, \
	pthread_mutex_t **forks)
{
	int i;

	*forks = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t) * parameters->nb_philo);
	i = 0;
	while (i < parameters->nb_philo)
	{
		pthread_mutex_init(&(*forks)[i], NULL);
		i++;
	}
	*philosophers = malloc(sizeof(t_philo) * parameters->nb_philo);
	i = 0;
	while (i < parameters->nb_philo)
	{
		(*philosophers)[i].id = i + 1;
		(*philosophers)[i].time_death = 100;
		(*philosophers)[i].nb_meal = 0;
		(*philosophers)[i].thread = (pthread_t)malloc(sizeof(pthread_t));
		(*philosophers)[i].params = parameters;
		(*philosophers)[i].forks = forks;
		pthread_create(&((*philosophers)[i].thread), NULL, philosophizing, &(*philosophers)[i]);
		i++;
	}
	parameters->start_time = get_time();
	i = 0;
	while (i < parameters->nb_philo)
	{
		pthread_join((*philosophers)[i].thread, NULL);
		i++;
	}
}

void	the_watcher(t_philo *philosophers)
{
	int	i;
	int full_philosophers;
	t_params *params;

	params = philosophers[0].params;
	usleep(params->time_die * 1000);
	i = 0;
	printf("yo");
	while (get_time() < philosophers[i].time_death)
	{
		printf("hello");
		printf("philo %d time death = %d, time now = %ld", philosophers[i].id, philosophers[i].time_death, get_time());
		if (philosophers[i].nb_meal == params->nb_meal)
			full_philosophers++;
		if (i < params->nb_philo)
			i++;
		else if (full_philosophers != params->nb_philo)
		{
			i = 0;
			full_philosophers = 0;
		}
		else
			break;
	}
	if (full_philosophers != params->nb_philo)
		message(philosophers[i].id, DIE, params->start_time, params->write_lock);
	return;
}

int		main(int argc, char **argv)
{
	t_params		parameters;
	t_philo			*philosophers;
	pthread_mutex_t	*forks;

	if (argc < 5 || argc > 6 || parser(&parameters, argv))
		return (usage(argv[0]));
	initialisation(&parameters, &philosophers, &forks);
	the_watcher(philosophers);
	return (0);
}
