#! /bin/bash

SESSION=coogle

tmux has-session -t $SESSION
if [ $? != 0 ]; then
	tmux new-session -d -s $SESSION

	tmux rename-window -t 0 'vim'
	tmux send-keys -t 'vim' 'nvim .' C-m

	tmux new-window -t $SESSION:1 -n 'bash'

	tmux new-window -t $SESSION:2 -n 'gdb'
	tmux send-keys -t 'gdb' 'gdb build/coogle'
fi

tmux attach-session -t $SESSION:0

